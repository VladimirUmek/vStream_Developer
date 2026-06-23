/*
 * Copyright 2026 Arm Limited and/or its affiliates.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "cmsis_vstream.h"

#include "cmsis_os2.h"
#include "app_main.h"
#include "app_setup.h"

/* Thread attributes */
const osThreadAttr_t thread_attr_main  = { .name = "app_main" };
const osThreadAttr_t thread_attr_stdin = { .name = "stdin"    };

/* Thread IDs */
osThreadId_t thread_id_main;
osThreadId_t thread_id_stdin;

/* Input and output stream buffers */
static uint8_t audio_in_buf[AUDIO_IN_BLOCK_SIZE   * AUDIO_IN_BUF_BLOCK_COUNT]  __attribute__((aligned(32)));
static uint8_t audio_out_buf[AUDIO_OUT_BLOCK_SIZE * AUDIO_OUT_BUF_BLOCK_COUNT] __attribute__((aligned(32)));

/* Thread flags */
#define FLAG_BLOCK_IN   (1U << 0)   ///< New input block available
#define FLAG_BLOCK_OUT  (1U << 1)   ///< Output block released by driver
#define FLAG_EOS        (1U << 2)   ///< End of stream (user request or driver event)

/*
  Thread that waits for stdin characters.
*/
__NO_RETURN void thread_stdin(void *argument) {
  int ch;

  (void)argument;

  printf("Press any key to stop streaming...\n");

  while (1) {
    ch = getchar();
    if (ch != EOF) {
      osThreadFlagsSet(thread_id_main, FLAG_EOS);
    }
    osDelay(100);
  }
}

/*
  Audio In Event Callback
*/
void AudioIn_Event_Callback(uint32_t event) {

  if (event & VSTREAM_EVENT_DATA) {
    osThreadFlagsSet(thread_id_main, FLAG_BLOCK_IN);
  }

  if (event & VSTREAM_EVENT_EOS) {
    osThreadFlagsSet(thread_id_main, FLAG_EOS);
  }
}

/*
  Audio Out Event Callback
*/
void AudioOut_Event_Callback(uint32_t event) {

  if (event & VSTREAM_EVENT_DATA) {
    osThreadFlagsSet(thread_id_main, FLAG_BLOCK_OUT);
  }
}

/*
  Initialize input and output streams.

  Registers event callbacks, allocates stream buffers, and pre-fills the
  first output block with silence so the output driver can start immediately.
*/
static void init_streams(void) {
  void *out_block;

  if (vStream_AudioIn->Initialize(AudioIn_Event_Callback) != VSTREAM_OK) {
    printf("Failed to initialize audio input\n");
    return;
  }
  if (vStream_AudioIn->SetBuf(audio_in_buf, sizeof(audio_in_buf), AUDIO_IN_BLOCK_SIZE) != VSTREAM_OK) {
    printf("Failed to set audio input buffer\n");
    return;
  }

  if (vStream_AudioOut->Initialize(AudioOut_Event_Callback) != VSTREAM_OK) {
    printf("Failed to initialize audio output\n");
    return;
  }
  if (vStream_AudioOut->SetBuf(audio_out_buf, sizeof(audio_out_buf), AUDIO_OUT_BLOCK_SIZE) != VSTREAM_OK) {
    printf("Failed to set audio output buffer\n");
    return;
  }

  /* Pre-fill the first output block with silence so the output stream can
   * be started immediately without waiting for the first processed block. */
  out_block = vStream_AudioOut->GetBlock();
  if (out_block != NULL) {
    memset(out_block, 0x00, AUDIO_OUT_BLOCK_SIZE);
    vStream_AudioOut->ReleaseBlock();
  }
}

/*
  Single-block streaming loop.

  Captures one block at a time: starts a single input capture, waits for the
  block, processes it, then triggers a single output. Exits when end of stream
  is signalled by the driver or the user.
*/
static void stream_single(void) {
  void           *in_block;
  void           *out_block;
  uint32_t        flags;
  vStreamStatus_t status;

  while (1) {
    if (vStream_AudioIn->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK) {
      printf("Failed to start audio input\n");
      return;
    }

    flags = osThreadFlagsWait(FLAG_BLOCK_IN | FLAG_EOS, osFlagsWaitAny, osWaitForever);
    if (flags & FLAG_EOS) {
      return;
    }

    in_block = vStream_AudioIn->GetBlock();
    if (in_block == NULL) {
      printf("Failed to get audio input block\n");
      return;
    }

    /* Wait for the output driver to finish with the previous block */
    do {
      status = vStream_AudioOut->GetStatus();
    } while (status.active == 1U);

    out_block = vStream_AudioOut->GetBlock();
    if (out_block == NULL) {
      printf("Failed to get audio output block\n");
      return;
    }

    memcpy(out_block, in_block, AUDIO_OUT_BLOCK_SIZE);

    if (vStream_AudioIn->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release audio input block\n");
    }
    if (vStream_AudioOut->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release audio output block\n");
    }

    if (vStream_AudioOut->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK) {
      printf("Failed to start audio output\n");
    }
  }
}

/*
  Continuous streaming loop.

  Starts both streams in continuous mode and processes blocks as they arrive,
  driven entirely by driver events. Exits when end of stream is signalled by
  the driver or the user.
*/
static void stream_continuous(void) {
  void    *in_block;
  void    *out_block;
  uint32_t flags;

  /* Start output first so playback begins with the pre-filled silence block */
  if (vStream_AudioOut->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK) {
    printf("Failed to start audio output\n");
    return;
  }
  if (vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK) {
    printf("Failed to start audio input\n");
    return;
  }

  while (1) {
    flags = osThreadFlagsWait(FLAG_BLOCK_IN | FLAG_EOS, osFlagsWaitAny, osWaitForever);
    if (flags & FLAG_EOS) {
      break;
    }

    in_block = vStream_AudioIn->GetBlock();
    if (in_block == NULL) {
      printf("Failed to get audio input block\n");
      break;
    }

    /* Wait for an output buffer to become available */
    osThreadFlagsWait(FLAG_BLOCK_OUT, osFlagsWaitAny, osWaitForever);

    out_block = vStream_AudioOut->GetBlock();
    if (out_block == NULL) {
      printf("Failed to get audio output block\n");
      break;
    }

    memcpy(out_block, in_block, AUDIO_OUT_BLOCK_SIZE);

    if (vStream_AudioIn->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release audio input block\n");
    }
    if (vStream_AudioOut->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release audio output block\n");
    }
  }
}

/*
  Application main thread.
*/
__NO_RETURN void app_main_thread(void *argument) {
  (void)argument;

  thread_id_stdin = osThreadNew(thread_stdin, NULL, &thread_attr_stdin);
  if (thread_id_stdin == NULL) {
    printf("Failed to create stdin thread\n");
  }

  init_streams();

#if (VSTREAM_CAPTURE_MODE == 0)
  stream_single();
#else
  stream_continuous();
#endif

  if (vStream_AudioIn->Stop() != VSTREAM_OK) {
    printf("Failed to stop audio input\n");
  }
  if (vStream_AudioOut->Stop() != VSTREAM_OK) {
    printf("Failed to stop audio output\n");
  }

  printf("Audio streaming stopped.\n");
  for (;;) {}
}

/*
  Application initialization.
*/
int app_main(void) {

  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize();

  /* Create application main thread */
  thread_id_main = osThreadNew(app_main_thread, NULL, &thread_attr_main);

  /* Start thread execution */
  osKernelStart();

  return 0;
}

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

#include "cmsis_vstream.h"

#include "cmsis_os2.h"
#include "app_main.h"
#include "vstream_layout.h"

/* Attributes for the app_main thread */
const osThreadAttr_t thread_attr_main  = { .name = "app_main" };

/* ID of the app_main thread */
osThreadId_t thread_id_main;

/* Input and output frame buffers */
static uint8_t video_in_buf[VIDEO_IN_FRAME_SIZE * VSTREAM_BUF_BLOCK_COUNT]   __attribute__((aligned(32), section(".bss.in_frame_buf")));
static uint8_t video_out_buf[VIDEO_OUT_FRAME_SIZE * VSTREAM_BUF_BLOCK_COUNT] __attribute__((aligned(32), section(".bss.out_frame_buf")));

/* Thread flags */
#define FLAG_FRAME_IN   (1U << 0)   ///< New input frame available
#define FLAG_FRAME_OUT  (1U << 1)   ///< Output frame buffer released by driver

/*
  vStream Video In Event Callback
*/
void VideoIn_Event_Callback(uint32_t event) {

  if (event & VSTREAM_EVENT_DATA) {
    osThreadFlagsSet(thread_id_main, FLAG_FRAME_IN);
  }
}

/*
  vStream Video Out Event Callback
*/
void VideoOut_Event_Callback(uint32_t event) {

  if (event & VSTREAM_EVENT_DATA) {
    osThreadFlagsSet(thread_id_main, FLAG_FRAME_OUT);
  }
}

/*
  Initialize input and output streams.

  Registers event callbacks, allocates stream buffers, and pre-fills the
  first output block with black so the output driver can start immediately.
*/
static void init_streams(void) {
  void *out_block;

  if (vStream_VideoIn->Initialize(VideoIn_Event_Callback) != VSTREAM_OK) {
    printf("Failed to initialize video input\n");
    return;
  }
  if (vStream_VideoIn->SetBuf(video_in_buf, sizeof(video_in_buf), VIDEO_IN_FRAME_SIZE) != VSTREAM_OK) {
    printf("Failed to set video input buffer\n");
    return;
  }

  if (vStream_VideoOut->Initialize(VideoOut_Event_Callback) != VSTREAM_OK) {
    printf("Failed to initialize video output\n");
    return;
  }
  if (vStream_VideoOut->SetBuf(video_out_buf, sizeof(video_out_buf), VIDEO_OUT_FRAME_SIZE) != VSTREAM_OK) {
    printf("Failed to set video output buffer\n");
    return;
  }

  /* Pre-fill the first output block with black so the output stream can
   * be started immediately without waiting for the first processed frame. */
  out_block = vStream_VideoOut->GetBlock();
  if (out_block != NULL) {
    memset(out_block, 0x00, VIDEO_OUT_FRAME_SIZE);
    vStream_VideoOut->ReleaseBlock();
  }
}

/*
  Single-frame streaming loop.

  Captures one frame at a time: starts a single input capture, waits for the
  frame, processes it, then triggers a single output. Runs indefinitely.
*/
static void stream_single(void) {
  void          *inFrame;
  void          *outFrame;
  vStreamStatus_t status;

  while (1) {
    if (vStream_VideoIn->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK) {
      printf("Failed to start video capture\n");
      return;
    }

    osThreadFlagsWait(FLAG_FRAME_IN, osFlagsWaitAny, osWaitForever);

    inFrame = vStream_VideoIn->GetBlock();
    if (inFrame == NULL) {
      printf("Failed to get video input frame\n");
      return;
    }

    /* Wait for the output driver to finish with the previous frame */
    do {
      status = vStream_VideoOut->GetStatus();
    } while (status.active == 1U);

    outFrame = vStream_VideoOut->GetBlock();
    if (outFrame == NULL) {
      printf("Failed to get video output frame\n");
      return;
    }

    process_frame(inFrame, outFrame);

    if (vStream_VideoIn->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release video input frame\n");
    }
    if (vStream_VideoOut->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release video output frame\n");
    }

    if (vStream_VideoOut->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK) {
      printf("Failed to start video output\n");
    }
  }
}

/*
  Continuous streaming loop.

  Starts both streams in continuous mode and processes frames as they arrive,
  driven entirely by driver events. Runs indefinitely.
*/
static void stream_continuous(void) {
  void *inFrame;
  void *outFrame;

  /* Start output first so the display receives the pre-filled black frame */
  if (vStream_VideoOut->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK) {
    printf("Failed to start video output\n");
    return;
  }
  if (vStream_VideoIn->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK) {
    printf("Failed to start video capture\n");
    return;
  }

  while (1) {
    osThreadFlagsWait(FLAG_FRAME_IN, osFlagsWaitAny, osWaitForever);

    inFrame = vStream_VideoIn->GetBlock();
    if (inFrame == NULL) {
      printf("Failed to get video input frame\n");
      return;
    }

    /* Wait for an output buffer to become available */
    osThreadFlagsWait(FLAG_FRAME_OUT, osFlagsWaitAny, osWaitForever);

    outFrame = vStream_VideoOut->GetBlock();
    if (outFrame == NULL) {
      printf("Failed to get video output frame\n");
      return;
    }

    process_frame(inFrame, outFrame);

    if (vStream_VideoIn->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release video input frame\n");
    }
    if (vStream_VideoOut->ReleaseBlock() != VSTREAM_OK) {
      printf("Failed to release video output frame\n");
    }
  }
}

/*
  Application main thread.
*/
__NO_RETURN void app_main_thread(void *argument) {
  (void)argument;

  init_streams();

#if (VSTREAM_CAPTURE_MODE == 0)
  stream_single();
#else
  stream_continuous();
#endif

  for (;;) {}
}

/*
  Application initialization.
*/
int app_main (void) {

  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize();

  /* Create application main thread */
  thread_id_main = osThreadNew(app_main_thread, NULL, &thread_attr_main);

  /* Start thread execution */
  osKernelStart();

  return 0;
}

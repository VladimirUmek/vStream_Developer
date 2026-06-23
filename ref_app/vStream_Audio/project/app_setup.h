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

#ifndef APP_SETUP_H_
#define APP_SETUP_H_

#include "vstream_config.h"

/*
 * vStream capture mode: map config value to CMSIS API constant
 */
#if   (VSTREAM_CAPTURE_MODE == 0)
  #define VSTREAM_MODE  VSTREAM_MODE_SINGLE
#elif (VSTREAM_CAPTURE_MODE == 1)
  #define VSTREAM_MODE  VSTREAM_MODE_CONTINUOUS
#else
  #error "vstream_config.h: VSTREAM_CAPTURE_MODE must be 0 (Single) or 1 (Continuous)"
#endif

/*
 * Stream buffer block count
 *   Single mode:     1 block per stream (one block in flight at a time)
 *   Continuous mode: 4 blocks per stream (ring buffer for pipeline overlap)
 */
#if (VSTREAM_CAPTURE_MODE == 0)
  #define AUDIO_IN_BUF_BLOCK_COUNT   1U
  #define AUDIO_OUT_BUF_BLOCK_COUNT  1U
#else
  #define AUDIO_IN_BUF_BLOCK_COUNT   4U
  #define AUDIO_OUT_BUF_BLOCK_COUNT  4U
#endif

/*
 * Block sizes in bytes
 *   One block holds one second of audio data.
 */
#define AUDIO_IN_BLOCK_SIZE   (AUDIO_IN_SAMPLE_RATE  * (AUDIO_IN_SAMPLE_BITS  / 8) * AUDIO_IN_CHANNELS)
#define AUDIO_OUT_BLOCK_SIZE  (AUDIO_OUT_SAMPLE_RATE * (AUDIO_OUT_SAMPLE_BITS / 8) * AUDIO_OUT_CHANNELS)

#endif /* APP_SETUP_H_ */

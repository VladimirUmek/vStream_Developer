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
#include "image_processing_func.h"

/*
 * Named constants for VIDEO_IN_FORMAT / VIDEO_OUT_FORMAT config values
 */
#define VIDEO_FORMAT_RAW8    0   ///< RAW Bayer, 1 byte/pixel (debayering applied)
#define VIDEO_FORMAT_RGB565  1   ///< 16-bit RGB: 5R 6G 5B, 2 bytes/pixel
#define VIDEO_FORMAT_RGB888  2   ///< 24-bit RGB: 8R 8G 8B, 3 bytes/pixel


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
 *   Single mode:     1 block per stream (one frame in flight at a time)
 *   Continuous mode: 4 blocks per stream (ring buffer for pipeline overlap)
 */
#if (VSTREAM_CAPTURE_MODE == 0)
  #define VSTREAM_BUF_BLOCK_COUNT  1U
#else
  #define VSTREAM_BUF_BLOCK_COUNT  4U
#endif


/*
 * Bytes-per-pixel and frame sizes
 */

/** Bytes per pixel for a given VIDEO_FORMAT_* value */
#define VIDEO_FORMAT_BPP(fmt) \
    ((fmt) == VIDEO_FORMAT_RAW8   ? 1 : \
     (fmt) == VIDEO_FORMAT_RGB565 ? 2 : 3)

/** Map VIDEO_FORMAT_* to image_format_t (valid for non-RAW8 formats only) */
#define VIDEO_FORMAT_TO_IMAGE_FORMAT(fmt) \
    ((fmt) == VIDEO_FORMAT_RGB565 ? IMAGE_FORMAT_RGB565 : IMAGE_FORMAT_RGB888)

#define VIDEO_IN_FRAME_BYTES    VIDEO_FORMAT_BPP(VIDEO_IN_FORMAT)
#define VIDEO_OUT_FRAME_BYTES   VIDEO_FORMAT_BPP(VIDEO_OUT_FORMAT)

#define VIDEO_IN_FRAME_SIZE     (VIDEO_IN_WIDTH  * VIDEO_IN_HEIGHT  * VIDEO_IN_FRAME_BYTES)
#define VIDEO_OUT_FRAME_SIZE    (VIDEO_OUT_WIDTH * VIDEO_OUT_HEIGHT * VIDEO_OUT_FRAME_BYTES)


/*
 * Layout selection
 *
 *  CROP   path  (IN area >= OUT area):
 *    Center-crop the largest sub-rectangle of the input that matches the
 *    output aspect ratio, then resize it to the output dimensions.
 *    The output frame is always fully covered — no black borders.
 *
 *  CENTER path  (OUT area > IN area):
 *    Place the input frame unscaled at the center of the output.
 *    Uncovered border areas are filled with black (0x00).
 */

#if ((VIDEO_IN_WIDTH) * (VIDEO_IN_HEIGHT)) >= ((VIDEO_OUT_WIDTH) * (VIDEO_OUT_HEIGHT))

  #define VSTREAM_LAYOUT_CROP   1

  #if ((VIDEO_IN_WIDTH) * (VIDEO_OUT_HEIGHT)) >= ((VIDEO_IN_HEIGHT) * (VIDEO_OUT_WIDTH))
    /* Input is relatively wider: remove left/right margins */
    #define CROP_H  (VIDEO_IN_HEIGHT)
    #define CROP_W  ((VIDEO_IN_HEIGHT) * (VIDEO_OUT_WIDTH) / (VIDEO_OUT_HEIGHT))
  #else
    /* Input is relatively taller: remove top/bottom margins */
    #define CROP_W  (VIDEO_IN_WIDTH)
    #define CROP_H  ((VIDEO_IN_WIDTH) * (VIDEO_OUT_HEIGHT) / (VIDEO_OUT_WIDTH))
  #endif

  /** Top-left corner of the center-crop window within the input frame */
  #define CROP_X  (((VIDEO_IN_WIDTH)  - (CROP_W)) / 2)
  #define CROP_Y  (((VIDEO_IN_HEIGHT) - (CROP_H)) / 2)

#else

  #define VSTREAM_LAYOUT_CENTER 1

  /** Top-left offset at which the unscaled input is placed in the output frame */
  #define CENTER_X  (((VIDEO_OUT_WIDTH)  - (VIDEO_IN_WIDTH))  / 2)
  #define CENTER_Y  (((VIDEO_OUT_HEIGHT) - (VIDEO_IN_HEIGHT)) / 2)

#endif

#endif /* APP_SETUP_H_ */

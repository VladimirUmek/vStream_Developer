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

#include <stdint.h>
#include <string.h>

#include "vstream_layout.h"
#include "image_processing_func.h"

/* Intermediate buffer for RAW8 debayering
 *   CROP path:   crop_and_debayer always produces RGB888  → 3 bytes/pixel at output resolution
 *   CENTER path: debayer directly to the output format    → VIDEO_OUT_FRAME_BYTES/pixel at input resolution
 */
#if (VIDEO_IN_FORMAT == VIDEO_FORMAT_RAW8)
  #if defined(VSTREAM_LAYOUT_CROP) && (VIDEO_OUT_FORMAT != VIDEO_FORMAT_RGB888)
  static uint8_t s_debayerBuf[VIDEO_OUT_WIDTH * VIDEO_OUT_HEIGHT * 3]                      __attribute__((aligned(32), section(".bss.rgb_frame_buf")));
  #elif defined(VSTREAM_LAYOUT_CENTER)
  static uint8_t s_debayerBuf[VIDEO_IN_WIDTH  * VIDEO_IN_HEIGHT  * VIDEO_OUT_FRAME_BYTES]  __attribute__((aligned(32), section(".bss.rgb_frame_buf")));
  #endif
#endif

/*
  Process one frame.

  CROP layout   (IN area >= OUT area):
    Center-crops a region of the input that matches the output aspect ratio,
    resizes it to the output dimensions. No black borders.

  CENTER layout (OUT area > IN area):
    Clears the output to black, then copies the input unscaled into the
    center of the output frame at (CENTER_X, CENTER_Y).
*/
void process_frame(const void *inFrame, void *outFrame) {

#if defined(VSTREAM_LAYOUT_CROP)

  #if (VIDEO_IN_FORMAT == VIDEO_FORMAT_RAW8)
    #if (VIDEO_OUT_FORMAT == VIDEO_FORMAT_RGB888)
    crop_and_debayer((const uint8_t *)inFrame,
                     VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                     CROP_X, CROP_Y,
                     (uint8_t *)outFrame,
                     VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT,
                     VIDEO_IN_BAYER_PATTERN);
    #else /* RGB565 output: debayer to RGB888 temp, then convert */
    crop_and_debayer((const uint8_t *)inFrame,
                     VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                     CROP_X, CROP_Y,
                     s_debayerBuf,
                     VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT,
                     VIDEO_IN_BAYER_PATTERN);
    convert_rgb888_to_rgb565(s_debayerBuf,
                             (uint8_t *)outFrame,
                             VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT);
    #endif
  #else /* RGB input */
  image_crop_and_resize((const uint8_t *)inFrame,
                        VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                        CROP_X, CROP_Y, CROP_W, CROP_H,
                        (uint8_t *)outFrame,
                        VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT,
                        VIDEO_FORMAT_TO_IMAGE_FORMAT(VIDEO_IN_FORMAT),
                        VIDEO_FORMAT_TO_IMAGE_FORMAT(VIDEO_OUT_FORMAT));
  #endif

#else /* VSTREAM_LAYOUT_CENTER: place input unscaled at the center of the output */

  /* Clear output to black (fills the border area around the centered input) */
  memset(outFrame, 0x00, VIDEO_OUT_FRAME_SIZE);

  #if (VIDEO_IN_FORMAT == VIDEO_FORMAT_RAW8)
    #if (VIDEO_OUT_FORMAT == VIDEO_FORMAT_RGB888)
    image_debayer((const uint8_t *)inFrame,
                  s_debayerBuf,
                  VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                  VIDEO_IN_BAYER_PATTERN, IMAGE_FORMAT_RGB888);
    image_copy_to_framebuffer(s_debayerBuf,
                              VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                              (uint8_t *)outFrame,
                              VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT,
                              CENTER_X, CENTER_Y,
                              IMAGE_FORMAT_RGB888);
    #else /* RGB565 output: debayer directly to RGB565, then copy */
    image_debayer((const uint8_t *)inFrame,
                  s_debayerBuf,
                  VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                  VIDEO_IN_BAYER_PATTERN, IMAGE_FORMAT_RGB565);
    image_copy_to_framebuffer(s_debayerBuf,
                              VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                              (uint8_t *)outFrame,
                              VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT,
                              CENTER_X, CENTER_Y,
                              IMAGE_FORMAT_RGB565);
    #endif
  #else /* RGB input: copy directly without scaling */
  image_copy_to_framebuffer((const uint8_t *)inFrame,
                            VIDEO_IN_WIDTH, VIDEO_IN_HEIGHT,
                            (uint8_t *)outFrame,
                            VIDEO_OUT_WIDTH, VIDEO_OUT_HEIGHT,
                            CENTER_X, CENTER_Y,
                            VIDEO_FORMAT_TO_IMAGE_FORMAT(VIDEO_IN_FORMAT));
  #endif

#endif /* VSTREAM_LAYOUT_CROP / VSTREAM_LAYOUT_CENTER */
}

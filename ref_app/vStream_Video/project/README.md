# vStream_Video Reference Application

The **vStream_Video** reference application demonstrates how to use the [CMSIS vStream](https://arm-software.github.io/CMSIS_6/)
driver interface to build a video pass-through pipeline on an Arm-based target. It reads frames from video-input peripheral
(camera), applies cropping or centering, and forwards the result to video-output peripheral (display).

## Operation

The application runs a single CMSIS-RTOS2 thread (`app_main`) that owns the full streaming pipeline:

1. **Initialization** – Both the video-input and video-output vStream drivers are initialized,
   their frame buffers are registered with `SetBuf()`, and the first output block is pre-filled
   with black so the display can be started immediately.

2. **Frame loop** – Depending on `VSTREAM_CAPTURE_MODE` (see [Configuration](#configuration)) one of two loops is entered:

   | Mode             | Description |
   |------------------|-------------|
   | **Single Frame** | `Start(VSTREAM_MODE_SINGLE)` is called once per frame on each stream. The thread waits on `FLAG_FRAME_IN` (set by the video-in event callback), retrieves the input block, waits for the output driver to become idle, fills the output block, releases both blocks, and triggers the output. |
   | **Continuous**   | Both streams are started once with `VSTREAM_MODE_CONTINUOUS`. The thread reacts to `FLAG_FRAME_IN` (new captured frame) and `FLAG_FRAME_OUT` (output buffer released) thread flags set by the respective event callbacks, keeping the pipeline running without stopping and restarting either stream. |

3. **Frame processing** – Each input frame is transformed into the output format by `process_frame()`.
   The transformation path is selected at compile-time based on the configured input/output resolutions and formats:

   | Layout      | Condition                | Behaviour |
   |-------------|--------------------------|-----------|
   | **Crop**    | Input area ≥ output area | The largest centered sub-rectangle of the input that matches the output aspect ratio is cropped and resized to fill the output completely — no black borders. |
   | **Center**  | Input area < output area | The input frame is placed unscaled at the center of the output. The surrounding border area is filled with black (0x00). |

   When the input format is **RAW8** (Bayer), the application automatically debayers the frame using the configured Bayer
   pattern before any format-conversion or copy step.

## Configuration

All run-time parameters are controlled by [`vstream_config.h`](vstream_config.h).

### Video Input

| Macro                    | Default      | Description                                                                  |
|--------------------------|--------------|------------------------------------------------------------------------------|
| `VIDEO_IN_FORMAT`        | `1` (RGB565) | Pixel format delivered by the input stream: `0`=RAW8, `1`=RGB565, `2`=RGB888 |
| `VIDEO_IN_WIDTH`         | `1280`       | Input frame width in pixels                                                  |
| `VIDEO_IN_HEIGHT`        | `720`        | Input frame height in pixels                                                 |
| `VIDEO_IN_BAYER_PATTERN` | `2` (GRBG)   | Bayer mosaic pattern: `0`=RGGB, `1`=BGGR, `2`=GRBG, `3`=GBRG                 |

> **Note:** `VIDEO_IN_BAYER_PATTERN` is only used when `VIDEO_IN_FORMAT` is RAW8.

### Video Output

| Macro              | Default      | Description                                                        |
|--------------------|--------------|--------------------------------------------------------------------|
| `VIDEO_OUT_FORMAT` | `1` (RGB565) | Pixel format expected by the output stream: `1`=RGB565, `2`=RGB888 |
| `VIDEO_OUT_WIDTH`  | `480`        | Output frame width in pixels                                       |
| `VIDEO_OUT_HEIGHT` | `800`        | Output frame height in pixels                                      |

### Streaming

| Macro                  | Default            | Description                                                          |
|------------------------|--------------------|----------------------------------------------------------------------|
| `VSTREAM_CAPTURE_MODE` | `0` (Single Frame) | Frame capture and display strategy: `0`=Single Frame, `1`=Continuous |

> **Note:** In **Continuous** mode the stream buffer is automatically sized to four blocks per stream (ring buffer)
> to allow pipeline overlap. In **Single Frame** mode one block per stream is sufficient.

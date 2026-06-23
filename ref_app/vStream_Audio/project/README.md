# vStream Audio Reference Application

The **vStream Audio** reference application demonstrates how to use the [CMSIS vStream](https://arm-software.github.io/CMSIS_6/)
driver interface to build an audio pass-through pipeline on an Arm-based target. It captures audio blocks from an
audio-input peripheral (microphone), copies them directly to an audio-output peripheral (speaker or DAC).

## Operation

The application runs two CMSIS-RTOS2 threads:

- **`app_main`** – owns the full streaming pipeline.
- **`stdin`** – monitors keyboard input and signals end of stream when any key is pressed.

The pipeline proceeds in three stages:

1. **Initialization** – Both the audio-input and audio-output vStream drivers are initialized,
   their buffers are registered with `SetBuf()`, and the first output block is pre-filled with
   silence so the output stream can be started immediately.

2. **Block loop** – Depending on `VSTREAM_CAPTURE_MODE` (see [Configuration](#configuration)) one of two loops is entered:

   | Mode             | Description |
   |------------------|-------------|
   | **Single Block** | `Start(VSTREAM_MODE_SINGLE)` is called once per block on the input. The thread waits on `FLAG_BLOCK_IN` (set by the audio-in event callback), retrieves the input block, polls `GetStatus()` until the output driver is idle, copies the input block to the output, releases both blocks, and triggers output with `Start(VSTREAM_MODE_SINGLE)`. |
   | **Continuous**   | Output is started first (playing the pre-filled silence block), then input is started, both with `VSTREAM_MODE_CONTINUOUS`. The thread reacts to `FLAG_BLOCK_IN` (new captured block) and `FLAG_BLOCK_OUT` (output buffer released) thread flags set by the respective event callbacks, keeping the pipeline running without stopping and restarting either stream. |

3. **Stopping** – Both loops exit when `FLAG_EOS` is raised. This flag is set either by the `stdin`
   thread (user keypress) or by the audio-input driver (`VSTREAM_EVENT_EOS`). After the loop exits,
   both streams are stopped and a confirmation message is printed.

## Configuration

All run-time parameters are controlled by [`vstream_config.h`](project/vstream_config.h).

### Audio Input

| Macro                  | Default          | Description                                                                 |
|------------------------|------------------|-----------------------------------------------------------------------------|
| `AUDIO_IN_CHANNELS`    | `2` (Stereo)     | Number of channels: `1`=Mono, `2`=Stereo                                    |
| `AUDIO_IN_SAMPLE_BITS` | `16`             | Bits per sample: `8`, `16`, `24`, or `32`                                   |
| `AUDIO_IN_SAMPLE_RATE` | `16000` (16 kHz) | Samples per second: `8000`, `16000`, `44100`, or `48000`                    |

### Audio Output

| Macro                   | Default          | Description                                                                 |
|-------------------------|------------------|-----------------------------------------------------------------------------|
| `AUDIO_OUT_CHANNELS`    | `2` (Stereo)     | Number of channels: `1`=Mono, `2`=Stereo                                    |
| `AUDIO_OUT_SAMPLE_BITS` | `16`             | Bits per sample: `8`, `16`, `24`, or `32`                                   |
| `AUDIO_OUT_SAMPLE_RATE` | `16000` (16 kHz) | Samples per second: `8000`, `16000`, `44100`, or `48000`                    |

### Streaming

| Macro                  | Default          | Description                                                           |
|------------------------|------------------|-----------------------------------------------------------------------|
| `VSTREAM_CAPTURE_MODE` | `1` (Continuous) | Audio capture and playback strategy: `0`=Single Block, `1`=Continuous |

> **Note:** In **Continuous** mode the stream buffer is automatically sized to four blocks per stream (ring buffer)
> to allow pipeline overlap. In **Single Block** mode one block per stream is sufficient.

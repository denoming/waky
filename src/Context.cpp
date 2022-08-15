#include "Context.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "states/DetectWakeWordState.hpp"
#include "states/RecordingCommandState.hpp"

Context::Context()
    : _state{nullptr}
{
}

void
Context::proceed(MemsMicrophone& sampler)
{
    static const TickType_t kMaxBlockTime = pdMS_TO_TICKS(100);

    _detectionState.reset(new DetectWakeWordState{sampler});
    _recordingState.reset(new RecordingCommandState{sampler});

    _state = _detectionState.get();

    assert(_state != nullptr);
    _state->enterState();

    while (true) {
        const uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, kMaxBlockTime);
        if (notificationValue == 0) {
            continue;
        }

        assert(_state != nullptr);
        if (_state->run()) {
            _state->exitState();
            if (_state == _detectionState.get()) {
                _state = _recordingState.get();
            } else {
                _state = _detectionState.get();
            }
            assert(_state != nullptr);
            _state->enterState();
        }
    }
}

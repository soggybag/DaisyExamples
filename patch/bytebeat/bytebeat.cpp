#include "daisy_patch.h"
#include "daisysp.h"
#include <stdio.h>  // Standard printf functions

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

float speed = 0.0f;
static uint32_t t = 0;  
static int formula_index = 0;  
static const int formula_count = 6;  

// Bytebeat formulas
uint8_t BytebeatFormula(uint32_t t, int index) {
    switch (index) {
        case 0: return (t * (t >> 10) & 42) | (t * (t >> 12) & 84);  
        case 1: return (t >> 6) | (t << 4) | (t * (t >> 8));  
        case 2: return (t * (t >> 7) | (t * (t >> 9) & 50));  
        case 3: return ((t * 3) & (t >> 5)) | (t * (t >> 3)); 
        case 4: return (t * 5 & (t >> 7)) | (t * (t >> 6) & 123);
        case 5: return (t >> 4) ^ (t * (t >> 5) & 32);
        default: return (t * (t >> 8) & 128) | (t * (t >> 12) & 64);
    }
}

// ✅ Ensure proper control updates
void UpdateControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();

    speed = patch.GetKnobValue(static_cast<DaisyPatch::Ctrl>(0)) * 4.0f + 0.1f;  // ✅ Read CTRL_1 properly

    if (patch.encoder.RisingEdge())
    {
        formula_index = (formula_index + 1) % formula_count;
    }
}

// ✅ Ensure controls are updated each cycle
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    UpdateControls();

    for (size_t i = 0; i < size; i++)
    {
        uint8_t sample = BytebeatFormula(t, formula_index);
        float output = ((float)sample / 255.0f) * 2.0f - 1.0f;
        out[0][i] = output;
        out[1][i] = output;
        t += (uint32_t)(1 + speed * 2);
    }
}

int main(void)
{
    patch.Init();
    patch.StartAdc();  // ✅ Required for analog inputs (fixes knob issues)
    patch.StartAudio(AudioCallback);

    while (1)
    {
        // ✅ OLED Debugging
        patch.display.Fill(false);
        patch.display.SetCursor(0, 0);
        patch.display.WriteString("Bytebeat Synth", Font_7x10, true);

        char buffer[32];  
        snprintf(buffer, sizeof(buffer), "Speed: %.2f", speed);
        patch.display.SetCursor(0, 12);
        patch.display.WriteString(buffer, Font_7x10, true);

        snprintf(buffer, sizeof(buffer), "Formula: %d", formula_index);
        patch.display.SetCursor(0, 24);
        patch.display.WriteString(buffer, Font_7x10, true);

        patch.display.Update();
    }
}
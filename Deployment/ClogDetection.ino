#include <Clog_Detection_inferencing.h>
#include "TFT_eSPI.h"


static TFT_eSPI tft;                   
static TFT_eSprite spr = TFT_eSprite(&tft);

volatile int  Pulse_Count;
unsigned int  flowrate;
unsigned long Current_Time, Loop_Time;
const int Output_Pin = D0;

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

uint8_t color[3][3] = {{0,255,0},{255,0,0},{0, 0, 255}};
char *label[] = {"Normal Flow","Clog !","No Flow"};

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}


void setup()
{
    Serial.begin(115200);
    //while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
    pinMode(Output_Pin, INPUT);
    attachInterrupt(0, Detect_Rising_Edge, RISING);

    tft.begin();
    tft.setRotation(3);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(&FreeSerifBold24pt7b);
    tft.fillScreen(tft.color565(98,0,234));
    
}

void updateFeatures() {
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += 2) {
     Current_Time = millis();
     if(Current_Time >= (Loop_Time + 500))
     {
        Loop_Time = Current_Time;
        flowrate = (Pulse_Count * 60 / 7.5);
        Pulse_Count = 0;
     }  
     features[i + 0] = flowrate;
     features[i + 1] = flowrate;
     delay(EI_CLASSIFIER_INTERVAL_MS);
    }
}

void Detect_Rising_Edge ()
{ 
   Pulse_Count++;
} 


void loop()
{
    ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(1000);
        return;
    }

    ei_impulse_result_t result = { 0 };

    updateFeatures();

    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    ei_printf("run_classifier returned: %d\n", res);

    if (res != 0) return;

    // print the predictions
    /*ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    ei_printf("[");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("%.5f", result.classification[ix].value);
        #if EI_CLASSIFIER_HAS_ANOMALY == 1
                ei_printf(", ");
        #else if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
                    ei_printf(", ");
                }
        #endif
    }
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("%.3f", result.anomaly);
    #endif
        ei_printf("]\n");
    */

    // human-readable predictions
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        if (result.classification[ix].value > 0.7){
          tft.fillScreen(tft.color565(color[ix][0],color[ix][1],color[ix][2]));
          tft.drawString(label[ix], 50, 90);
          
        }
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("anomaly score: %.3f\n", result.anomaly);
#endif

    delay(100);
}

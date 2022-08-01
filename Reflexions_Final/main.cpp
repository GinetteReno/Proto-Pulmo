#include <stdio.h>

#include "sendosc.h"
#include "serialib.h"
#include "slmx4_vcom.h"
#include "Proto_Pulmo.h"

#define RUN_TIME 10

#define DEBUG


#define FULL
#ifdef FULL
int main()
{
	slmx4 sensor;

	sensor.Begin();

	sensor.Iterations();

	sensor.TryUpdateChip(slmx4::rx_wait);
	sensor.TryUpdateChip(slmx4::frame_start);
	sensor.TryUpdateChip(slmx4::frame_end);
	sensor.TryUpdateChip(slmx4::ddc_en);
	
	timeOut run_time;
	run_time.initTimer();

while(run_time.elapsedTime_ms() < 1000 * RUN_TIME)
	sensor.GetFrameNormalized();

	sensor.End();

	return 0;
}


#else
int main()
{
	slmx4 sensor;
	//sensor.init_serial();

	sensor.Begin();
	sensor.End();
}
#endif


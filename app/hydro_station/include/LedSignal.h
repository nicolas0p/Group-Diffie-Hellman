#ifndef INTERFACE_H_
#define INTERFACE_H_

using namespace EPOS;

class Interface{
	public:
		enum ERROR{NOSIMCARD,NONETWORK};

		Interface(){
			led = new GPIO{'c', 3, GPIO::OUTPUT};
		}

		void showLife(){
			blink(100);
			led->set(1);
		}

		void blinkError(ERROR type){
			switch(type){
				case NOSIMCARD:
					blink(20);
					break;
				case NONETWORK:
					blink(40);
				}
		}

	private:
		GPIO *led;

		void blink(int x){
			for (auto i = 0u; i < x; ++i) {
				led->set();
				eMote3_GPTM::delay(50000);
				eMote3_GPTM::delay(50000);
			}
			led->set(0);
		}

};

#endif

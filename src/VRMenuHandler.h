#ifndef VRMENUHANDLER_H
#define VRMENUHANDLER_H


class VRMenuElement;

	class VRMenuHandler {
	public:
		VRMenuHandler(){};
		virtual ~VRMenuHandler(){};

		virtual void handleEvent(VRMenuElement * element) = 0 ;
	};

#endif //VRMENUHANDLER_H
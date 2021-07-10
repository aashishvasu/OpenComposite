#pragma once

namespace ocapi {
	namespace IVROCSystem_001 {

		// Use ButtonMaskFromId on these, like you would with EVRButtonId
		enum EVRExtendedButtonId {
			k_EButton_OVRMenu = 0,
		};

		class IVROCSystem {
		public:
			/**
			* Gets the extended button status
			*
			* This is for buttons inaccessable from OpenVR - currently, this is just the menu
			*  button (the left Touch's seperate menu button, not any of the four standard buttons).
			*
			* This is packed in the same manner as OpenVR's button values, so you need to use ButtonMaskFromId
			* to get the values out (you'll have to cast EVRExtendedButtonId to EVRButtonId though).
			*
			* Eg. if(sys->GetExtendedButtonStatus() & ButtonMaskFromId((EVRButtonId) k_EButton_OVRMenu)) ...
			*/
			virtual uint64_t GetExtendedButtonStatus() = 0;
		};

		static const char * const IVROCSystem_Version = "IVROCSystem_001";

	}
}

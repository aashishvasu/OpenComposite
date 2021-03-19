#pragma once
#if ANDROID

// Symbols used to exchange critical information between OpenComposite and the application

extern "C" {
extern XrInstanceCreateInfoAndroidKHR* OpenComposite_Android_Create_Info;
extern XrGraphicsBindingOpenGLESAndroidKHR* OpenComposite_Android_GLES_Binding_Info;
};

extern std::string (*OpenComposite_Android_Load_Input_File)(const char* path);

#endif

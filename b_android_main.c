#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "betray.h"
#include "imagine.h"
#include <jni.h>
#include <errno.h>
#include <math.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>

#include <android/input.h>
#include <android/log.h>
#include <android/api-level.h>
#include <android/native_window.h>
#include <android/window.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "b_android_native_app_glue.h"

static boolean	input_focus = TRUE;
//static int	b_window_size_x = 800;
//static int	b_window_size_y = 600;
static int	b_window_center_x = 400;
static int	b_window_center_y = 300;
static int	b_window_pos_x = 800;
static int	b_window_pos_y = 600;

struct android_app* betray_android_app_state;

struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};

uint betray_android_pointer_allocations[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };


typedef struct 
{
	JavaVM 		vm;
	jobject 	activity;
	const char* app_dir;
} AssetThreadAttributes;

typedef struct{
	struct android_app* app;
	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	const ASensor* gyroscopicSensor;
	ASensorEventQueue* sensorEventQueue;
	uint device_id;
	uint axis_up;
	uint axis_forward;
	uint accelerometer;
	uint pointers[16];
	float axis_matrix[16];
	float last_accelerometer[3];
	float delta_accelerometer[3];
	char app_dir[256];
	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig config;
	EGLint format;
	pthread_t asset_thread;
	int32_t width;
	int32_t height;
	struct saved_state state;
}BetrayAndroidState;

BetrayAndroidState betray_android_state = {};

uint betray_plugin_axis_allocate(uint user_id, uint device_id, char *name, BAxisType type, uint axis_count);
void betray_plugin_axis_set(uint id, float axis_x, float axis_y, float axis_z);

extern void betray_plugin_callback_main(BInputState *input);
extern void betray_plugin_pointer_clean();
extern void betray_action(BActionMode mode);
extern void betray_reshape_view(uint x_size, uint y_size);
extern uint betray_plugin_pointer_allocate(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw);
extern void betray_plugin_pointer_set(uint id, float x, float y, float z, float *origin, boolean *buttons);
extern void betray_plugin_pointer_free(uint id);
extern void betray_plugin_button_set(uint user_id, uint id, boolean state, uint character);

void betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer)
{

}

void betray_screen_mode_safe_get(BetraySafeArea safe_area, float *left, float *right, float *top, float *bottom)
{
	//FK: TODO
	const float aspect = (float)betray_android_state.height / (float)betray_android_state.width;
	*left = -1.0f;
	*right = 1.0f;
	*top = aspect;
	*bottom = -aspect;
}

boolean b_win32_system_wrapper_set_display(uint size_x, uint size_y, boolean full_screen) 
{  
}

uint betray_support_functionality(BSupportedFunctionality funtionality)
{
	uint array[] = {
				1, /*B_SF_USER_COUNT_MAX*/
				16, /*B_SF_POINTER_COUNT_MAX*/
				1, /*B_SF_POINTER_BUTTON_COUNT*/
				TRUE, /*B_SF_FULLSCREEN*/
				FALSE, /*B_SF_WINDOWED*/
				FALSE, /*B_SF_VIEW_POSITION*/
				FALSE, /*B_SF_VIEW_ROTATION*/
				FALSE, /*B_SF_MOUSE_WARP*/
				FALSE, /*B_SF_EXECUTE*/
				FALSE, /*B_SF_REQUESTER*/
				TRUE}; /*B_SF_CLIPBOARD*/
	if(funtionality >= B_SF_COUNT)
		return FALSE;
	return array[funtionality];
}



void betray_requester_save_func(void *data)
{
}
char *betray_requester_save_get(void *id)
{
	return NULL;
}

void betray_requester_save(char **types, uint type_count, void *id)
{
}

void betray_requester_load_func(void *data)
{
}

char *betray_requester_load_get(void *id)
{
	return NULL;
}

void betray_requester_load(char **types, uint type_count, void *id)
{
}

char *betray_clipboard_get()
{
	return NULL;
}

void betray_clipboard_set(char *text)
{
}

void betray_url_launch(char *url)
{

	JavaVM lJavaVM = *betray_android_app_state->activity->vm;
	JNIEnv* env;
	int result;
	env = NULL;
	result = lJavaVM->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);
	jstring url_string = (*env)->NewStringUTF(env, url);
	jclass uri_class = (*env)->FindClass(env, "android/net/Uri");
	jmethodID uri_parse = (*env)->GetStaticMethodID(env, uri_class, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
	jobject uri = (*env)->CallStaticObjectMethod(env, uri_class, uri_parse, url_string);
	jclass intent_class = (*env)->FindClass(env, "android/content/Intent");
	jfieldID action_view_id = (*env)->GetStaticFieldID(env, intent_class, "ACTION_VIEW", "Ljava/lang/String;");
	jobject action_view = (*env)->GetStaticObjectField(env, intent_class, action_view_id);
	jmethodID new_intent = (*env)->GetMethodID(env, intent_class, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
	jobject intent = (*env)->AllocObject(env, intent_class);
	(*env)->CallVoidMethod(env, intent, new_intent, action_view, uri);
	jclass activity_class = (*env)->FindClass(env, "android/app/Activity");
	jmethodID start_activity = (*env)->GetMethodID(env, activity_class, "startActivity", "(Landroid/content/Intent;)V");
	(*env)->CallVoidMethod(env, betray_android_app_state->activity->clazz, start_activity, intent);
	result = 0;
}
/*
void get_network()
{

	JavaVM lJavaVM = *betray_android_app_state->activity->vm;
	JNIEnv* env;
	int result;
	env = NULL;
	result = lJavaVM->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);
	jclass InterfaceAddress = (*env)->FindClass(env, "java/net/InterfaceAddress");
	jmethodID hashCode = (*env)->GetMethodID(env, InterfaceAddress, "hashCode", "()I");
	result = (*env)->CallIntMethod(env, InterfaceAddress, hashCode);
	result = 0;
}*/

int betray_android_unicode_convert(int eventType, int keyCode, int metaState)
{
	JavaVM lJavaVM = *betray_android_app_state->activity->vm;
	JNIEnv* env;
	env = NULL;
	int result;
	result = lJavaVM->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);
	jclass class_key_event = (*env)->FindClass(env, "android/view/KeyEvent");
	int unicodeKey;
	if(metaState == 0)
	{
		jmethodID method_get_unicode_char = (*env)->GetMethodID(env, class_key_event, "getUnicodeChar", "()I");
		jmethodID eventConstructor = (*env)->GetMethodID(env, class_key_event, "<init>", "(II)V");
		jobject eventObj = (*env)->NewObject(env, class_key_event, eventConstructor, eventType, keyCode);
		unicodeKey = (*env)->CallIntMethod(env, eventObj, method_get_unicode_char);
	}else
	{
		jmethodID method_get_unicode_char = (*env)->GetMethodID(env, class_key_event, "getUnicodeChar", "(I)I");
		jmethodID eventConstructor = (*env)->GetMethodID(env, class_key_event, "<init>", "(II)V");
		jobject eventObj = (*env)->NewObject(env, class_key_event, eventConstructor, eventType, keyCode);
		unicodeKey = (*env)->CallIntMethod(env, eventObj, method_get_unicode_char, metaState);
	}
	return unicodeKey;
}



void betray_device_init()
{
}

void b_win32_window_close()
{

}

void betray_desktop_size_get(uint *size_x, uint *size_y) 
{
	*size_x = 0;
}

 
boolean betray_activate_context(void *context)
{
}

void *b_create_context() 
{
}

boolean b_init_display_opengl(uint size_x, uint size_y, boolean fullscreenflag, uint samples, char* title, boolean *sterioscopic) 
{

}

#ifdef BETRAY_CONTEXT_OPENGLES


void betray_button_keyboard(uint user_id, boolean show)
{
	// Attaches the current thread to the JVM.
	jint lResult;
	jint lFlags = 1;
	JavaVM *vm;
	JNIEnv* env;
	int result;
	JavaVM lJavaVM = *betray_android_app_state->activity->vm;
	env = NULL;
	result = lJavaVM->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);


	vm = betray_android_app_state->activity->vm;

	JavaVMAttachArgs lJavaVMAttachArgs;
	lJavaVMAttachArgs.version = JNI_VERSION_1_6;
	lJavaVMAttachArgs.name = "NativeThread";
	lJavaVMAttachArgs.group = NULL;
//	lResult = betray_android_app_state->activity->vm[0]->AttachCurrentThread(betray_android_app_state->activity->vm, betray_android_app_state->activity->env, &lJavaVMAttachArgs);
//	if(lResult == -1)
//		return;

	// Retrieves NativeActivity.
	jobject lNativeActivity = betray_android_app_state->activity->clazz;
	jclass ClassNativeActivity = (*env)->GetObjectClass(env, lNativeActivity);

	// Retrieves Context.INPUT_METHOD_SERVICE.
	jclass ClassContext = (*env)->FindClass(env, "android/content/Context");
	jfieldID FieldINPUT_METHOD_SERVICE = (*env)->GetStaticFieldID(env, ClassContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
	jobject INPUT_METHOD_SERVICE = (*env)->GetStaticObjectField(env, ClassContext,FieldINPUT_METHOD_SERVICE);
//	jniCheck(INPUT_METHOD_SERVICE);

	// Runs getSystemService(Context.INPUT_METHOD_SERVICE).
	jclass ClassInputMethodManager = (*env)->FindClass(env, "android/view/inputmethod/InputMethodManager");
	jmethodID MethodGetSystemService = (*env)->GetMethodID(env, ClassNativeActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
	jobject lInputMethodManager = (*env)->CallObjectMethod(env, lNativeActivity, MethodGetSystemService, INPUT_METHOD_SERVICE);

	// Runs getWindow().getDecorView().
	jmethodID MethodGetWindow = (*env)->GetMethodID(env, ClassNativeActivity, "getWindow", "()Landroid/view/Window;");
	jobject lWindow = (*env)->CallObjectMethod(env, lNativeActivity, MethodGetWindow);
	jclass ClassWindow = (*env)->FindClass(env, "android/view/Window");
	jmethodID MethodGetDecorView = (*env)->GetMethodID(env, ClassWindow, "getDecorView", "()Landroid/view/View;");
	jobject lDecorView = (*env)->CallObjectMethod(env, lWindow, MethodGetDecorView);

	if(show)
	{
		// Runs lInputMethodManager.showSoftInput(...).
		jmethodID MethodShowSoftInput = (*env)->GetMethodID(env, ClassInputMethodManager, "showSoftInput", "(Landroid/view/View;I)Z");
		lResult = (*env)->CallBooleanMethod(env, lInputMethodManager, MethodShowSoftInput, lDecorView, lFlags);
	}else
	{
		// Runs lWindow.getViewToken()
		jclass ClassView = (*env)->FindClass(env, "android/view/View");
		jmethodID MethodGetWindowToken = (*env)->GetMethodID(env, ClassView, "getWindowToken", "()Landroid/os/IBinder;");
		jobject lBinder = (*env)->CallObjectMethod(env, lDecorView, MethodGetWindowToken);

		// lInputMethodManager.hideSoftInput(...).
		jmethodID MethodHideSoftInput = (*env)->GetMethodID(env, ClassInputMethodManager, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
		jboolean lRes = (*env)->CallBooleanMethod(env, lInputMethodManager, MethodHideSoftInput, lBinder, lFlags);
	}
	show = 0;
	// Finished with the JVM.
//	betray_android_app_state->activity->vm[0]->DetachCurrentThread(betray_android_app_state->activity->vm);
}

int betray_create_directory_tree( const char* pBaseDir, const char* pDirTree )
{
	char tempDirectoryPath[512] = {0};
	strcat( tempDirectoryPath, pBaseDir );
	strcat( tempDirectoryPath, "/" );

	const size_t baseDirLength = strlen( pBaseDir );
	const size_t dirTreeLength = strlen( pDirTree );
	for( size_t dirTreeIndex = 0u; dirTreeIndex < dirTreeLength; ++dirTreeIndex )
	{
		tempDirectoryPath[ baseDirLength + 1 + dirTreeIndex ] = pDirTree[ dirTreeIndex ];

		boolean callMkDir = false;
		
		if( pDirTree[ dirTreeIndex ] == '/' )
		{
			tempDirectoryPath[ baseDirLength + 1 + dirTreeIndex + 1] = 0;
			callMkDir = true;
		}
		else if( dirTreeIndex + 1 == dirTreeLength )
		{
			tempDirectoryPath[ baseDirLength + 1 + dirTreeIndex + 1 ] = '/';
			tempDirectoryPath[ baseDirLength + 1 + dirTreeIndex + 2 ] = 0;
			callMkDir = true;
		}

		if( callMkDir )
		{
			const int result = mkdir( tempDirectoryPath, S_IRWXU );
			if( result == -1 )
			{
				if( errno != EEXIST )
				{
					return -1;
				}
			}
		}
	}

	return 1;
}

void betray_recreate_asset_directory_tree_recursively( const char* pAssetDirectory, const char* pApplicationDirectory, JNIEnv* env, jobject assetManager, jmethodID listAssets )
{
	jobject assetDir = (*env)->NewStringUTF(env, pAssetDirectory);
	jarray assetList = (jarray)(*env)->CallObjectMethod(env, assetManager, listAssets, assetDir);
	jsize assetListLength = 0u;
	jsize assetIndex = 0u;

	char assetPath[512] = {0};
	strcat( assetPath, pAssetDirectory );

	if( strlen( assetPath ) > 0 )
	{
		strcat( assetPath, "/");
	}	

	const size_t assetBasePathPos = strlen( assetPath );
	assetListLength = (*env)->GetArrayLength(env, assetList);

	for( assetIndex = 0u; assetIndex < assetListLength; ++assetIndex )
	{
		jobject assetName = (*env)->GetObjectArrayElement(env, assetList, assetIndex);
		const char* pAssetName = (*env)->GetStringUTFChars(env, assetName, NULL);
		assetPath[ assetBasePathPos ] = 0;
		strcat( assetPath, pAssetName );

		char applicationAssetPath[512] = {0};
		strcat( applicationAssetPath, pApplicationDirectory );
		strcat( applicationAssetPath, "/" );
		strcat( applicationAssetPath, assetPath );

		//FK: Try to open asset, if this fails this is a directory
		AAsset* pAsset = AAssetManager_open(betray_android_app_state->activity->assetManager, assetPath, AASSET_MODE_STREAMING);
		if( pAsset == NULL )
		{
			__android_log_print( ANDROID_LOG_INFO, "betray", "Creating directory \"%s\" to recreate asset directory tree", applicationAssetPath );
			if( betray_create_directory_tree( pApplicationDirectory, assetPath ) == -1 )
			{
				__android_log_print( ANDROID_LOG_ERROR, "betray", "Couldn't create directory \"%s\". errno:%d", applicationAssetPath, errno );
				continue;
			}
			
			betray_recreate_asset_directory_tree_recursively( assetPath, pApplicationDirectory, env, assetManager, listAssets );
		}
		else
		{
			__android_log_print( ANDROID_LOG_INFO, "betray", "Dumping asset \"%s\"...", assetPath );
			FILE* out = fopen(applicationAssetPath, "wb");
			if( out == NULL )
			{
				__android_log_print( ANDROID_LOG_ERROR, "betray",  "Couldn't dump asset \"%s\", errno:%d", assetPath, errno );
				continue;
			}

			char buf[4096];
			int nb_read = 0;
			while((nb_read = AAsset_read(pAsset, buf, 4096)) > 0)
				fwrite(buf, nb_read, 1, out);
			
			fclose(out);
			AAsset_close(pAsset);
		}

		(*env)->ReleaseStringUTFChars(env, assetName, pAssetName);
	}

	(*env)->DeleteLocalRef(env, assetDir);
}

void* betray_thread_recreate_asset_directory_tree(void* params)
{
	AssetThreadAttributes* thread_attributes = (AssetThreadAttributes*)params;

	JNIEnv* env;
	thread_attributes->vm->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);
	jclass activityClass = (*env)->FindClass(env, "android/app/NativeActivity");

	jmethodID getAssets = (*env)->GetMethodID(env, activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
	jobject assetManager = (*env)->CallObjectMethod(env, thread_attributes->activity, getAssets);

	jclass assetManagerClass = (*env)->FindClass(env, "android/content/res/AssetManager");
	jmethodID listAssets = (*env)->GetMethodID(env, assetManagerClass, "list", "(Ljava/lang/String;)[Ljava/lang/String;");

	betray_recreate_asset_directory_tree_recursively("", thread_attributes->app_dir, env, assetManager, listAssets);
	thread_attributes->vm->DetachCurrentThread(betray_android_app_state->activity->vm);

}

void betray_get_android_app_dir(char* out_app_dir, jobject activity, JavaVM* vm)
{
	JNIEnv* env;
	(*vm)->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);

	jclass activityClass = (*env)->FindClass(env, "android/app/NativeActivity");
	jmethodID getCacheDir = (*env)->GetMethodID(env, activityClass, "getFilesDir", "()Ljava/io/File;");
	jobject file = (*env)->CallObjectMethod(env, activity, getCacheDir);
	jclass fileClass = (*env)->FindClass(env, "java/io/File");
	jmethodID getAbsolutePath = (*env)->GetMethodID(env, fileClass, "getAbsolutePath", "()Ljava/lang/String;");
	jstring jpath = (jstring)(*env)->CallObjectMethod(env, file, getAbsolutePath);

	const char* app_dir = (*env)->GetStringUTFChars(env, jpath, NULL);
	const size_t app_dir_length = strlen( app_dir );
	memcpy( out_app_dir, app_dir, app_dir_length );
	out_app_dir[ app_dir_length ] = 0;

	(*env)->ReleaseStringUTFChars(env, jpath, app_dir);
	(*vm)->DetachCurrentThread(betray_android_app_state->activity->vm);
}

void betray_set_directory()
{
	const char expected_sentinel_content[] = "zenith.sentinel.v1";
	char app_dir[256] = {0};
	betray_get_android_app_dir( app_dir, betray_android_app_state->activity->clazz, betray_android_app_state->activity->vm );

	//FK: Check if sentinel file is accessible - if it is, it means we have already dumped the asset folder
	char sentinel_file_path[512] = {0};
	strcat( sentinel_file_path, app_dir );
	strcat( sentinel_file_path, "/.sentinel" );

	__android_log_print( ANDROID_LOG_INFO, "betray", "Trying to search for sentinel file at '%s'.", sentinel_file_path );
	FILE* sentinel_file = fopen( sentinel_file_path, "r" );
	boolean dump_assets = ( sentinel_file == NULL );

	if( sentinel_file != NULL )
	{
		__android_log_print( ANDROID_LOG_INFO, "betray", "Found sentinel file, checking content..." );
		
		char sentinel_content[64];
		fread( sentinel_content, 1, sizeof( expected_sentinel_content ), sentinel_file );
		if( memcmp( sentinel_content, expected_sentinel_content, sizeof( expected_sentinel_content ) ) != 0 )
		{
			__android_log_print( ANDROID_LOG_WARN, "betray", "Content of sentinel file not equal to what was expected, re-dumping assets..." );

			//FK: content differs from what is expected, dump assets again...
			dump_assets = TRUE;
		}
		else
		{
			__android_log_print( ANDROID_LOG_INFO, "betray", "Content of sentinel file is equal to what was expected, no need to re-dump assets..." );
		}

		fclose( sentinel_file );
	}

	if( dump_assets )
	{
		//FK: Sentinel file not present - dump asset dir
		//FK: Recreate directory tree from asset folder in case there is available functionality that relies on an existing directory structure
		AssetThreadAttributes* thread_attributes = (AssetThreadAttributes*)malloc( sizeof( AssetThreadAttributes ) );
		thread_attributes->activity	= betray_android_app_state->activity->clazz;
		thread_attributes->vm 		= *betray_android_app_state->activity->vm;
		thread_attributes->app_dir	= app_dir;
		pthread_create( &betray_android_state.asset_thread, NULL, betray_thread_recreate_asset_directory_tree, thread_attributes );

		//FK: TODO: Waiting for the thread synchonously doesn't make much sense here.
		// 	  I have to figure out where the best place for this would be...
		pthread_join( betray_android_state.asset_thread, NULL );

		//FK: Create sentinel file after dumping all assets
		__android_log_print( ANDROID_LOG_INFO, "betray", "Trying to create sentinel file at '%s'...", sentinel_file_path );

		FILE* sentinel_file = fopen( sentinel_file_path, "w" );
		if( sentinel_file != NULL )
		{
			__android_log_print( ANDROID_LOG_INFO, "betray", "Successfully created sentinel file, writing content..." );
			fwrite( expected_sentinel_content, 1, sizeof( expected_sentinel_content ), sentinel_file );
			fclose( sentinel_file );
		}
		else
		{
			__android_log_print( ANDROID_LOG_ERROR, "betray", "Couldn't create sentinel file, unfortunately assets have to get re-dumped next time... (errno:%d)", errno );
		}

		free(thread_attributes);
	}
	
	chdir(app_dir);
}


void betray_immersive_mode()
{
	boolean success = TRUE;
	int version;	
	JNIEnv* env;
	int result, api_version;
	JavaVM lJavaVM = *betray_android_app_state->activity->vm;
	env = NULL;

//	api_version = android_get_device_api_level();
	ANativeActivity_setWindowFlags(betray_android_app_state->activity, AWINDOW_FLAG_FULLSCREEN/*AWINDOW_FLAG_FULLSCREEN*/, AWINDOW_FLAG_FORCE_NOT_FULLSCREEN);

	lJavaVM->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);
	version = (*env)->GetVersion(env);
	jclass activityClass = (*env)->FindClass(env, "android/app/NativeActivity");
	jclass windowClass = (*env)->FindClass(env, "android/view/Window");
	jclass viewClass = (*env)->FindClass(env, "android/view/View");
	jmethodID getWindow = (*env)->GetMethodID(env, activityClass, "getWindow", "()Landroid/view/Window;");
	jmethodID getDecorView = (*env)->GetMethodID(env, windowClass, "getDecorView", "()Landroid/view/View;");
	jmethodID setSystemUiVisibility = (*env)->GetMethodID(env, viewClass, "setSystemUiVisibility", "(I)V");
	jmethodID getSystemUiVisibility = (*env)->GetMethodID(env, viewClass, "getSystemUiVisibility", "()I");
	jmethodID setFitsSystemWindows = (*env)->GetMethodID(env, viewClass, "setFitsSystemWindows", "(Z)V");

//	(*env)->CallVoidMethodA(env, viewClass, setFitsSystemWindows, &boolean_value);

/*	jclass layout_params = env->FindClass(env, "android/WindowManager/LayoutParams");
	jfieldID id_LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES = env->GetStaticFieldID(env, windowClass, "LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES", "I");
	jclass visibilityClass = env->FindClass(env, "android/view/View");
*/
	jobject windowObj = (*env)->CallObjectMethod(env, betray_android_app_state->activity->clazz, getWindow);
	jobject decorViewObj = (*env)->CallObjectMethod(env, windowObj, getDecorView);

	jvalue boolean_value;
	boolean_value.z = FALSE;
	(*env)->CallVoidMethod(env, decorViewObj, setFitsSystemWindows, (uint8)1);
	// Get flag ids
	jfieldID id_SYSTEM_UI_FLAG_LAYOUT_STABLE = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_LAYOUT_STABLE", "I");
	jfieldID id_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION", "I");
	jfieldID id_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN", "I");
	jfieldID id_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I");
	jfieldID id_SYSTEM_UI_FLAG_FULLSCREEN = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I");
	jfieldID id_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I");
	jfieldID id_SYSTEM_UI_FLAG_IMMERSIVE = (*env)->GetStaticFieldID(env, viewClass, "SYSTEM_UI_FLAG_IMMERSIVE", "I");

	// Get flags
	const int flag_SYSTEM_UI_FLAG_LAYOUT_STABLE = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_STABLE);
	const int flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
	const int flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
	const int flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_HIDE_NAVIGATION);
	const int flag_SYSTEM_UI_FLAG_FULLSCREEN = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_FULLSCREEN);
	const int flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
	const int flag_SYSTEM_UI_FLAG_IMMERSIVE = (*env)->GetStaticIntField(env, viewClass, id_SYSTEM_UI_FLAG_IMMERSIVE);
	
	// Get current immersiveness
	int currentVisibility = (*env)->CallIntMethod(env, decorViewObj, getSystemUiVisibility);
	bool is_SYSTEM_UI_FLAG_LAYOUT_STABLE = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_STABLE) != 0;
	bool is_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION) != 0;
	bool is_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN) != 0;
	bool is_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (currentVisibility & flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION) != 0;
	bool is_SYSTEM_UI_FLAG_FULLSCREEN = (currentVisibility & flag_SYSTEM_UI_FLAG_FULLSCREEN) != 0;
	bool is_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (currentVisibility & flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY) != 0;

	const auto isAlreadyImmersive =
		is_SYSTEM_UI_FLAG_LAYOUT_STABLE &&
		is_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION &&
		is_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN &&
		is_SYSTEM_UI_FLAG_HIDE_NAVIGATION &&
		is_SYSTEM_UI_FLAG_FULLSCREEN &&
		is_SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
/*	PORTIS_LOGD()
		<< "set_immersive data"
		<< is_SYSTEM_UI_FLAG_LAYOUT_STABLE
		<< is_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
		<< is_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
		<< is_SYSTEM_UI_FLAG_HIDE_NAVIGATION
		<< is_SYSTEM_UI_FLAG_FULLSCREEN
		<< is_SYSTEM_UI_FLAG_IMMERSIVE_STICKY;*/
	
	if (true) {
		const int flag =
			flag_SYSTEM_UI_FLAG_LAYOUT_STABLE |
			flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
			flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
			flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
			flag_SYSTEM_UI_FLAG_FULLSCREEN |
			flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
			0;
		(*env)->CallVoidMethod(env, decorViewObj, setSystemUiVisibility, flag);
		if((*env)->ExceptionCheck(env)) 
		{
			// Read exception msg
			jthrowable e = (*env)->ExceptionOccurred(env);
			(*env)->ExceptionClear(env); // clears the exception; e seems to remain valid
			jclass clazz = (*env)->GetObjectClass(env, e);
			jmethodID getMessage = (*env)->GetMethodID(env, clazz, "getMessage", "()Ljava/lang/String;");
			jstring message = (jstring)(*env)->CallObjectMethod(env, e, getMessage);
			const char *mstr = (*env)->GetStringUTFChars(env, message, NULL);
		//	const auto exception_msg = std::string{mstr};
			(*env)->ReleaseStringUTFChars(env, message, mstr);
			(*env)->DeleteLocalRef(env, message);
			(*env)->DeleteLocalRef(env, clazz);
			(*env)->DeleteLocalRef(env, e);
			success = FALSE;
		}
	}

	currentVisibility = (*env)->CallIntMethod(env, decorViewObj, getSystemUiVisibility);
	is_SYSTEM_UI_FLAG_LAYOUT_STABLE = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_STABLE) != 0;
	is_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION) != 0;
	is_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN) != 0;
	is_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (currentVisibility & flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION) != 0;
	is_SYSTEM_UI_FLAG_FULLSCREEN = (currentVisibility & flag_SYSTEM_UI_FLAG_FULLSCREEN) != 0;
	is_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (currentVisibility & flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY) != 0;



	jmethodID addFlags = (*env)->GetMethodID(env, windowClass, "addFlags", "(I)V");
	(*env)->CallVoidMethod(env, windowObj, addFlags, 0x00000400 /*FLAG_FULLSCREEN*/ & 
								   0x00800000 /*FLAG_IMMERSIVE */);
	jmethodID clearFlags = (*env)->GetMethodID(env, windowClass, "clearFlags", "(I)V");
	(*env)->CallVoidMethod(env, windowObj, clearFlags, 0x00000800 /*FLAG_FORCE_NOT_FULLSCREEN*/);
/*	jmethodID setNavigationBarColor = (*env)->GetMethodID(env, windowClass, "setNavigationBarColor", "(I)V");
	(*env)->CallVoidMethod(env, windowObj, setNavigationBarColor, 0xAA883388);
	jmethodID setStatusBarColor = (*env)->GetMethodID(env, windowClass, "setStatusBarColor", "(I)V");
	(*env)->CallVoidMethod(env, windowObj, setStatusBarColor, 0xAA883388);
*/


	static const int WindowManager_LayoutParams_LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES = 0x00000001;
	jmethodID getAttributes = (*env)->GetMethodID(env, windowClass, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
	jobject attributes = (*env)->CallObjectMethod(env, windowObj, getAttributes);
	jclass clazz = (*env)->GetObjectClass(env, attributes);
	jfieldID mi = (*env)->GetFieldID(env, clazz, "layoutInDisplayCutoutMode", "I");
	(*env)->SetIntField(env, attributes, mi, WindowManager_LayoutParams_LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES);

/*
	Window window = getWindow();
	window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS); // 0x80000000
	window.setStatusBarColor(Color.BLUE);	
	*/

	(*env)->DeleteLocalRef(env, windowObj);
	(*env)->DeleteLocalRef(env, decorViewObj);

/*	{
		jmethodID getWindow = env->GetMethodID(env, activityClass, "getWindow", "()Landroid/view/Window;");

		static const int WindowManager_LayoutParams_LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES = 0x00000001;
		jobject window = _glfmCallJavaMethod(env, iandroid_app->activity->clazz, "getWindow", "()Landroid/view/Window;", Object);
		jobject attributes = _glfmCallJavaMethod(env, window, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;", Object);
		jclass clazz = env->GetObjectClass(env, attributes);
		jfieldID mi = env->GetFieldID(env, clazz, "layoutInDisplayCutoutMode", "I");

		env->SetIntField(env, attributes, mi, WindowManager_LayoutParams_LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES);
		env->DeleteLocalRef(env, clazz);
		env->DeleteLocalRef(env, attributes);
		env->DeleteLocalRef(env, window);
	}*/
/*

WindowManager.LayoutParams lp = getWindow().getAttributes();
lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES; 
getWindow().setAttributes(lp);


protected void onCreate(Bundle savedInstanceState)
{
	if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
	{
		WindowManager.LayoutParams layoutParams = new WindowManager.LayoutParams();
		layoutParams.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
		getWindow().setAttributes(layoutParams);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
	}
	super.onCreate(savedInstanceState);
}
*/


//	betray_url_launch("http://www.google.com");
	success = TRUE;
}

#endif    

void system_wrapper_lose_focus(void)
{
	input_focus = FALSE;
}

void betray_set_mouse_warp(boolean warp)
{
}


void betray_set_mouse_move(float x, float y)
{
}

uint betray_support_context(BContextType context_type)
{
	return context_type == B_CT_OPENGLES2 || context_type == B_CT_OPENGL_OR_ES;
}

extern uint BGlobal_draw_state_fbo;
/*
void APIENTRY betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer)
{
	static void (APIENTRY *internal_glBindFramebufferEXT)(GLenum target, GLuint framebuffer) = NULL;
	if(internal_glBindFramebufferEXT == NULL)
		internal_glBindFramebufferEXT = (void (APIENTRY __stdcall *)(GLenum , GLuint))wglGetProcAddress("glBindFramebufferEXT");
	if(framebuffer == 0)
		internal_glBindFramebufferEXT(target, BGlobal_draw_state_fbo);
	else
		internal_glBindFramebufferEXT(target, framebuffer);
} 

void *betray_gl_proc_address_get_internal(const char *text)
{
#ifdef BETRAY_CONTEXT_OPENGL
	if(b_win32_opengl_context_current == b_win32_opengl_context)
	{
		uint i;
		char *extension = "glBindFramebuffer";
		for(i = 0; extension[i] == text[i] && extension[i] != 0; i++);
		if(extension[i] == 0)
			return betray_glBindFramebufferEXT;
	}
	return wglGetProcAddress(text);
#endif
}
*/
void *betray_gl_proc_address_get()
{
	return (void *)eglGetProcAddress;
}

extern void betray_time_update(void);
int my_nCmdShow;

/**
* Initialize an EGL context for the current display.
*/

void betray_immersive_mode();

static int engine_init_surface(BetrayAndroidState *state)
{
	state->surface = eglCreateWindowSurface(state->display, state->config, state->app->window, NULL);
	if(state->surface == EGL_NO_SURFACE)
		return 0;

	eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	ANativeWindow_setBuffersGeometry(state->app->window, 0, 0, state->format);
	eglQuerySurface(state->display, state->surface, EGL_WIDTH, &state->width);
	eglQuerySurface(state->display, state->surface, EGL_HEIGHT, &state->height);

	return 1;
}

static void engine_term_surface(BetrayAndroidState* state)
{
	if( state->surface != EGL_NO_SURFACE )
	{
		state->width = 0;
		state->height = 0;

		eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, state->context);
		eglDestroySurface(state->display, state->surface);
		state->surface = EGL_NO_SURFACE;
	}
}

static int engine_init_context(BetrayAndroidState *state)
{
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
	//	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLint aEGLContextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint w, h, format, i;
	EGLint numConfigs;
	EGLConfig config[16];
	EGLSurface surface;
	EGLContext context;

//	betray_immersive_mode();
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);/* EGL_MULTISAMPLE_RESOLVE_DEFAULT */
	
	//sqrt((double)d_sqrt(3894576));
	if(!eglInitialize(display, 0, 0))
		exit(0);
	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	if(!eglChooseConfig(display, attribs, &config, 16, &numConfigs))
		exit(0);
	for(i = 0; i < numConfigs; i++)
	{
		EGLint x, y, z;
		eglGetConfigAttrib(display, config[i], EGL_DEPTH_SIZE, &x);
		eglGetConfigAttrib(display, config[i], EGL_HEIGHT, &y);
	}
	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	if(!eglGetConfigAttrib(display, config[0], EGL_NATIVE_VISUAL_ID, &format))
		exit(0);

	context = eglCreateContext(display, config[0], EGL_NO_CONTEXT, aEGLContextAttributes);

	betray_android_state.display = display;
	betray_android_state.context = context;
	betray_android_state.config	= config[0];
	betray_android_state.format	= format;
	betray_android_state.state.angle = 0;
	betray_android_state.axis_matrix[0] = 1;
	betray_android_state.axis_matrix[1] = 0;
	betray_android_state.axis_matrix[2] = 0;
	betray_android_state.axis_matrix[3] = 0;
	betray_android_state.axis_matrix[4] = 0;
	betray_android_state.axis_matrix[5] = 1;
	betray_android_state.axis_matrix[6] = 0;
	betray_android_state.axis_matrix[7] = 0;
	betray_android_state.axis_matrix[8] = 0;
	betray_android_state.axis_matrix[9] = 0;
	betray_android_state.axis_matrix[10] = 1;
	betray_android_state.axis_matrix[11] = 0;
	betray_android_state.axis_matrix[12] = 0;
	betray_android_state.axis_matrix[13] = 0;
	betray_android_state.axis_matrix[14] = 0;
	betray_android_state.axis_matrix[15] = 1;
	betray_android_state.last_accelerometer[0] = 0;
	betray_android_state.last_accelerometer[1] = 0;
	betray_android_state.last_accelerometer[2] = 0;
	betray_android_state.delta_accelerometer[0] = 0;
	betray_android_state.delta_accelerometer[1] = 0;
	betray_android_state.delta_accelerometer[2] = 0;

	if( engine_init_surface(state) == FALSE )
		exit(0);

	betray_reshape_view(betray_android_state.width, betray_android_state.height);
	return 0;
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_context(BetrayAndroidState *state)
{
	if(state->display != EGL_NO_DISPLAY)
	{
		engine_term_surface(state);
		if(state->context != EGL_NO_CONTEXT)
			eglDestroyContext(state->display, state->context);
		eglTerminate(state->display);
	}
	state->animating = 0;
	state->display = EGL_NO_DISPLAY;
	state->context = EGL_NO_CONTEXT;
	state->surface = EGL_NO_SURFACE;
//	Cube_tearDownGL();
}



static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	struct engine* engine = (struct engine*)app->userData;
	BInputState *input;
	uint count, i, j, id, max_pointer_count;
	uint32 key_val, unicode;
	input = betray_get_input_state();
	max_pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	switch(AInputEvent_getType(event))
	{
		case AINPUT_EVENT_TYPE_MOTION:
			{
				count = AMotionEvent_getPointerCount(event);
				for(i = 0; i < count && i < betray_support_functionality(B_SF_POINTER_COUNT_MAX); i++)
				{
					id = AMotionEvent_getPointerId(event, i);
					for(j = 0; j < max_pointer_count && betray_android_state.pointers[j] != id; j++);
					if(j == max_pointer_count)
					{
						j = betray_plugin_pointer_allocate(0, betray_android_state.device_id, 1,
							2.0 * (AMotionEvent_getX(event, i) - ((float)betray_android_state.width / 2.0)) / (float)betray_android_state.width,
							-2.0 * (AMotionEvent_getY(event, i) - ((float)betray_android_state.height / 2.0)) / (float)betray_android_state.width,
							-1, NULL, "Touch", FALSE);
						if(j < max_pointer_count)
							betray_android_state.pointers[j] = id;
					}
					if(j < max_pointer_count)
					{
						BInputPointerState *p;
						float origin[3] = {0, 0, 0};
						uint action;
						boolean button;
						button = input->pointers[j].button[0];
						action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
						if(action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP)
							button = FALSE;
						if(action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN)
							button = TRUE;
						betray_plugin_pointer_set(j, 
												2.0 * (AMotionEvent_getX(event, i) - ((float)betray_android_state.width / 2.0)) / (float)betray_android_state.width,
												-2.0 * (AMotionEvent_getY(event, i) - ((float)betray_android_state.height / 2.0)) / (float)betray_android_state.width,
												-1, origin, &button);
					}
				}
			}
			break;
		case AINPUT_EVENT_TYPE_KEY:
			key_val = (uint32)AKeyEvent_getKeyCode(event);
			unicode = betray_android_unicode_convert(AINPUT_EVENT_TYPE_KEY, key_val, AKeyEvent_getMetaState(event));
			if(unicode == 0)
				unicode = -1;
			if(AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
				betray_plugin_button_set(0, key_val, TRUE, unicode);
			else
				betray_plugin_button_set(0, key_val, FALSE, unicode);
			break;
	}
	return 1;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
	struct engine* engine = (struct engine*)app->userData;
//	printf("command %u\n", cmd);
	switch(cmd)
	{
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			betray_android_state.app->savedState = malloc(sizeof(struct saved_state));
			*((struct saved_state*)betray_android_state.app->savedState) = betray_android_state.state;
			betray_android_state.app->savedStateSize = sizeof(struct saved_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			if(betray_android_state.context == NULL)
			{
				engine_init_context(&betray_android_state);
			}
			else
			{
				engine_init_surface(&betray_android_state);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			engine_term_surface(&betray_android_state);
			break;
		case APP_CMD_WINDOW_RESIZED : 
		{
			uint x_size, y_size;
			eglQuerySurface(betray_android_state.display, betray_android_state.surface, EGL_WIDTH, &x_size);
			eglQuerySurface(betray_android_state.display, betray_android_state.surface, EGL_HEIGHT, &y_size);
			betray_reshape_view(x_size, y_size);
		}
		break;
	/*	case APP_CMD_WINDOW_RESIZED : 
		{
			uint x_size, y_size;
			eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &x_size);
			eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &y_size);
			betray_reshape_view(x_size, y_size);
		}
		break; */
		case APP_CMD_GAINED_FOCUS:
			// When our app gains focus, we start monitoring the accelerometer.
			if(betray_android_state.accelerometerSensor != NULL)
			{
				ASensorEventQueue_enableSensor(betray_android_state.sensorEventQueue, betray_android_state.accelerometerSensor);
				ASensorEventQueue_setEventRate(betray_android_state.sensorEventQueue, betray_android_state.accelerometerSensor, (1000L / 60) * 1000);
			}
			if(betray_android_state.gyroscopicSensor != NULL)
			{
				ASensorEventQueue_enableSensor(betray_android_state.sensorEventQueue, betray_android_state.gyroscopicSensor);
				ASensorEventQueue_setEventRate(betray_android_state.sensorEventQueue, betray_android_state.gyroscopicSensor, (1000L / 60) * 1000);
			}
			betray_android_state.animating = TRUE;
		break;
		case APP_CMD_LOST_FOCUS:
			// When our app loses focus, we stop monitoring the accelerometer.
			// This is to avoid consuming battery while not being used.
			if(betray_android_state.accelerometerSensor != NULL)
				ASensorEventQueue_disableSensor(betray_android_state.sensorEventQueue, betray_android_state.accelerometerSensor);
			if(betray_android_state.gyroscopicSensor != NULL)
				ASensorEventQueue_disableSensor(betray_android_state.sensorEventQueue, betray_android_state.gyroscopicSensor);
			// Also stop animating.
			betray_android_state.animating = FALSE;
		break;
	}
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/



void android_start(struct android_app* state)
{
	
}

boolean betray_get_package_name( char* pTarget, size_t targetCapacity) 
{
	JavaVM lJavaVM = *betray_android_app_state->activity->vm;
	JNIEnv* env = NULL;

	lJavaVM->AttachCurrentThread(betray_android_app_state->activity->vm, &env, NULL);
	jobject activity = betray_android_app_state->activity->clazz;
    jclass activityClass = (*env)->GetObjectClass( env, activity );

    jmethodID getPackageNameMethod = (*env)->GetMethodID( env, activityClass, "getPackageName", "()Ljava/lang/String;" );
    jstring packageName = (jstring)(*env)->CallObjectMethod( env, activity, getPackageNameMethod );

	const char* pPackageName = (*env)->GetStringUTFChars( env, packageName, 0 );
	const size_t packageNameLength = strlen( pPackageName );

	const boolean canCopy = ( packageNameLength + 1 > targetCapacity );
	if( canCopy )
	{
		memcpy( pTarget, pPackageName, packageNameLength + 1 );
	}

	(*env)->ReleaseStringUTFChars( env, packageName, pPackageName );
    return canCopy;
  }

boolean b_android_init_display(uint *window_size_x, uint *window_size_y, char *name)
{
	char packageName[256];
	betray_get_package_name( packageName, sizeof( packageName ) );

//	get_network();
	memset(&betray_android_state, 0, sizeof(betray_android_state));

	//FK: TODO: Check if a mouse is present and get correct id...
	const uint max_pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	for( uint pointer_index = 0u; pointer_index < max_pointer_count; ++pointer_index )
	{
		betray_android_state.pointers[ pointer_index ] = -1;
	}

	betray_android_app_state->userData = &betray_android_state;
	betray_android_app_state->onAppCmd = engine_handle_cmd;
	betray_android_app_state->onInputEvent = engine_handle_input;
	betray_android_state.surface = EGL_NO_SURFACE;
	betray_android_state.app = betray_android_app_state;
	// Prepare to monitor accelerometer	;
	betray_android_state.device_id = betray_plugin_input_device_allocate(betray_plugin_user_allocate(), "Device");
	betray_android_state.sensorManager = ASensorManager_getInstanceForPackage( packageName );
	betray_android_state.accelerometerSensor = ASensorManager_getDefaultSensor(betray_android_state.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	betray_android_state.gyroscopicSensor = ASensorManager_getDefaultSensor(betray_android_state.sensorManager, ASENSOR_TYPE_GYROSCOPE);
	if(betray_android_state.gyroscopicSensor != NULL)
	{
		betray_android_state.axis_up = betray_plugin_axis_allocate(0, betray_android_state.device_id, "Orientation up", B_AXIS_SCREEN_UP, 3);
		betray_android_state.axis_forward = betray_plugin_axis_allocate(0, betray_android_state.device_id, "Orinentation forward", B_AXIS_SCREEN_FORWARD, 3);
	}
	if(betray_android_state.accelerometerSensor != NULL)
	{
		betray_android_state.accelerometer = betray_plugin_axis_allocate(0, betray_android_state.device_id, "Accelerometer", 	B_AXIS_SCREEN_ACCELEROMETER, 3);
	}

	betray_android_state.sensorEventQueue = ASensorManager_createEventQueue(betray_android_state.sensorManager, betray_android_app_state->looper, LOOPER_ID_USER, NULL, NULL);
	if(betray_android_app_state->savedState != NULL)
	{
		// We are starting with a previous saved state; restore from it.
		betray_android_state.state = *(struct saved_state*)betray_android_app_state->savedState;
	}
	betray_android_state.animating = 1;
	
	while(betray_android_state.surface == EGL_NO_SURFACE)
	{
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		if((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
		{
			if (source != NULL)
				source->process(betray_android_app_state, source);
			if (betray_android_app_state->destroyRequested != 0)
				return FALSE;
		}
	}
	*window_size_x = betray_android_state.width;
	*window_size_y = betray_android_state.height;
	betray_reshape_view(betray_android_state.width, betray_android_state.height);
	return TRUE;
}

void betray_launch_main_loop(void)
{
	BInputState *input;
	input = betray_get_input_state();
	while(TRUE)
	{
		uint i, j;
		int ident;
		int events;
		float vector[3], counter[3], m[16];
		struct android_poll_source* source;

		while((ident = ALooper_pollAll(betray_android_state.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
		{
			if(source != NULL)
				source->process(betray_android_app_state, source);
			if(ident == LOOPER_ID_USER)
			{
				ASensorEvent event;
				while(ASensorEventQueue_getEvents(betray_android_state.sensorEventQueue, &event, 1) > 0)
				{
					switch(event.type)
					{	
						case ASENSOR_TYPE_ACCELEROMETER :
							betray_plugin_axis_set(betray_android_state.accelerometer, event.acceleration.x, event.acceleration.y, event.acceleration.z);
						/*	f_matrix_reverse4f(m, engine.axis_matrix); 
							vector[0] = m[4] * 0.995 + 0.005 * event.acceleration.x;
							vector[1] = m[5] * 0.995 + 0.005 * event.acceleration.y;
							vector[2] = m[6] * 0.995 + 0.005 * event.acceleration.z;
							counter[0] = m[8];
							counter[1] = m[9];
							counter[2] = m[10];
							f_matrixyzf(m, NULL, vector, counter);
							f_matrix_reverse4f(engine.axis_matrix, m); 
							betray_plugin_axis_set(engine.axis_up, engine.axis_matrix[4], engine.axis_matrix[5], engine.axis_matrix[6]);
							betray_plugin_axis_set(engine.axis_forward, engine.axis_matrix[8], engine.axis_matrix[9], engine.axis_matrix[10]);*/
						break;
						case ASENSOR_TYPE_GYROSCOPE :
							vector[0] = betray_android_state.axis_matrix[4] + event.uncalibrated_gyro.x_uncalib * betray_android_state.axis_matrix[8] * 0.02 - event.uncalibrated_gyro.z_uncalib * betray_android_state.axis_matrix[0] * 0.02;
							vector[1] = betray_android_state.axis_matrix[5] + event.uncalibrated_gyro.x_uncalib * betray_android_state.axis_matrix[9] * 0.02 - event.uncalibrated_gyro.z_uncalib * betray_android_state.axis_matrix[1] * 0.02;
							vector[2] = betray_android_state.axis_matrix[6] + event.uncalibrated_gyro.x_uncalib * betray_android_state.axis_matrix[10] * 0.02 - event.uncalibrated_gyro.z_uncalib * betray_android_state.axis_matrix[2] * 0.02;
							
							counter[0] = betray_android_state.axis_matrix[8] + event.uncalibrated_gyro.y_uncalib * betray_android_state.axis_matrix[0] * 0.02;
							counter[1] = betray_android_state.axis_matrix[9] + event.uncalibrated_gyro.y_uncalib * betray_android_state.axis_matrix[1] * 0.02;
							counter[2] = betray_android_state.axis_matrix[10] + event.uncalibrated_gyro.y_uncalib * betray_android_state.axis_matrix[2] * 0.02;

							f_matrixyzf(betray_android_state.axis_matrix, NULL, vector, counter);
							betray_plugin_axis_set(betray_android_state.axis_up, betray_android_state.axis_matrix[4], betray_android_state.axis_matrix[5], betray_android_state.axis_matrix[6]);
							betray_plugin_axis_set(betray_android_state.axis_forward, betray_android_state.axis_matrix[8], betray_android_state.axis_matrix[9], betray_android_state.axis_matrix[10]);
						break;
					}
				}
			}
			if(betray_android_app_state->destroyRequested != 0)
				return;
		}

		betray_time_update();
		if(betray_android_state.animating)
		{
			BInputState *input;
			input = betray_get_input_state();
			if(betray_android_state.animating)
				betray_action(BAM_EVENT);
			if(betray_android_state.display != NULL)
			{
				betray_action(BAM_DRAW);
				eglSwapBuffers(betray_android_state.display, betray_android_state.surface);
			}
			input->frame_number++;
		//	betray_event_reset(input);
		}
		betray_action(BAM_MAIN);
		input->button_event_count = 0;
		for(i = 0; i < 16; i++)
		{
			if(betray_android_state.pointers[i] != -1)
			{
				input->pointers[i].delta_pointer_x = 0;
				input->pointers[i].delta_pointer_y = 0;
				for(j = 0; j < input->pointers[i].button_count; j++)
					input->pointers[i].last_button[j] = input->pointers[i].button[j];
			}
		}
	}
}

extern int main(int argc, char **argv);

void android_main(struct android_app* state)
{
	betray_android_app_state = state;
//	android_start(state);
	char *argument = "file.exe";
	
	betray_set_directory();
	betray_immersive_mode();
	main(1, &argument);
	engine_term_context(&betray_android_state);
}


#include "ofConstants.h"
#include "ofSystemUtils.h"
#include "ofFileUtils.h"
#include "ofLog.h"
#include "ofUtils.h"
#include "ofAppRunner.h"
#include <condition_variable>
#include <mutex>

#ifdef TARGET_WIN32
#include <winuser.h>
#include <commdlg.h>
#define _WIN32_DCOM

#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#include <stdio.h>

#endif

#ifdef TARGET_OSX
	// ofSystemUtils.cpp is configured to build as
	// objective-c++ so as able to use Cocoa dialog panels
	// This is done with this compiler flag
	//		-x objective-c++
	// http://www.yakyak.org/viewtopic.php?p=1475838&sid=1e9dcb5c9fd652a6695ac00c5e957822#p1475838

	#include <Cocoa/Cocoa.h>
#endif

#ifdef TARGET_WIN32
#include <locale>
#include <sstream>
#include <string>

std::string convertWideToNarrow( const wchar_t *s, char dfault = '?',
                      const std::locale& loc = std::locale() )
{
  std::ostringstream stm;

  while( *s != L'\0' ) {
    stm << std::use_facet< std::ctype<wchar_t> >( loc ).narrow( *s++, dfault );
  }
  return stm.str();
}

std::wstring convertNarrowToWide( const std::string& as ){
    // deal with trivial case of empty string
    if( as.empty() )    return std::wstring();

    // determine required length of new string
    size_t reqLength = ::MultiByteToWideChar( CP_UTF8, 0, as.c_str(), (int)as.length(), 0, 0 );

    // construct new string of required length
    std::wstring ret( reqLength, L'\0' );

    // convert old string to new string
    ::MultiByteToWideChar( CP_UTF8, 0, as.c_str(), (int)as.length(), &ret[0], (int)ret.length() );

    // return new string ( compiler should optimize this away )
    return ret;
}

#endif

#if defined( TARGET_OSX )
static void restoreAppWindowFocus(){
	NSWindow * appWindow = (NSWindow *)ofGetCocoaWindow();
	if(appWindow) {
		[appWindow makeKeyAndOrderFront:nil];
	}
}
#endif

#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)
#include <gtk/gtk.h>
#include "ofGstUtils.h"
#include "Poco/Condition.h"

#if GTK_MAJOR_VERSION>=3
#define OPEN_BUTTON "_Open"
#define SELECT_BUTTON "_Select All"
#define SAVE_BUTTON "_Save"
#define CANCEL_BUTTON "_Cancel"
#else
#define OPEN_BUTTON GTK_STOCK_OPEN
#define SELECT_BUTTON GTK_STOCK_SELECT_ALL
#define SAVE_BUTTON GTK_STOCK_SAVE
#define CANCEL_BUTTON GTK_STOCK_CANCEL
#endif

gboolean init_gtk(gpointer userdata){
	int argc=0; char **argv = nullptr;
	gtk_init (&argc, &argv);

	return FALSE;
}

struct FileDialogData{
	GtkFileChooserAction action;
	string windowTitle;
	string defaultName;
	string results;
	bool done;
	Poco::Condition condition;
	std::mutex mutex;
};

gboolean file_dialog_gtk(gpointer userdata){
	FileDialogData * dialogData = (FileDialogData*)userdata;
	const gchar* button_name = nullptr;
	switch(dialogData->action){
	case GTK_FILE_CHOOSER_ACTION_OPEN:
		button_name = OPEN_BUTTON;
		break;
	case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:
		button_name = SELECT_BUTTON;
		break;
	case GTK_FILE_CHOOSER_ACTION_SAVE:
		button_name = SAVE_BUTTON;
		break;
	default:
		break;
	}

	if(button_name!=nullptr){
		GtkWidget *dialog = gtk_file_chooser_dialog_new (dialogData->windowTitle.c_str(),
							  nullptr,
							  dialogData->action,
							  button_name, GTK_RESPONSE_ACCEPT,
							  CANCEL_BUTTON, GTK_RESPONSE_CANCEL,
							  nullptr);

		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),dialogData->defaultName.c_str());

		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
			dialogData->results = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		}
		gtk_widget_destroy (dialog);
	}

	dialogData->mutex.lock();
	dialogData->condition.signal();
	dialogData->done = true;
	dialogData->mutex.unlock();
	return FALSE;
}

struct TextDialogData{
	string text;
	string question;
	bool done;
	std::condition_variable condition;
	std::mutex mutex;
};

gboolean alert_dialog_gtk(gpointer userdata){
	TextDialogData * dialogData = (TextDialogData*)userdata;
	GtkWidget* dialog = gtk_message_dialog_new (nullptr, (GtkDialogFlags) 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", dialogData->text.c_str());
	gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK));
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	dialogData->mutex.lock();
	dialogData->condition.notify_all();
	dialogData->done = true;
	dialogData->mutex.unlock();

	return FALSE;
}

gboolean text_dialog_gtk(gpointer userdata){
	TextDialogData * dialogData = (TextDialogData*)userdata;
	GtkWidget* dialog = gtk_message_dialog_new (nullptr, (GtkDialogFlags) 0, GTK_MESSAGE_QUESTION, (GtkButtonsType) GTK_BUTTONS_OK_CANCEL, "%s", dialogData->question.c_str() );
	GtkWidget* content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	GtkWidget* textbox = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(textbox),dialogData->text.c_str());
	gtk_container_add (GTK_CONTAINER (content_area), textbox);
	gtk_widget_show_all (dialog);
	if(gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_OK){
		dialogData->text = gtk_entry_get_text(GTK_ENTRY(textbox));
	}
	gtk_widget_destroy (dialog);
	dialogData->mutex.lock();
	dialogData->condition.notify_all();
	dialogData->done = true;
	dialogData->mutex.unlock();

	return FALSE;
}

static void initGTK(){
	static bool initialized = false;
	if(!initialized){
		#if !defined(TARGET_RASPBERRY_PI) 
		XInitThreads();
		#endif
		int argc=0; char **argv = nullptr;
		gtk_init (&argc, &argv);
		ofGstUtils::startGstMainLoop();
		initialized = true;
	}

}

static string gtkFileDialog(GtkFileChooserAction action,string windowTitle,string defaultName=""){
	initGTK();
	FileDialogData dialogData;
	dialogData.action = action;
	dialogData.windowTitle = windowTitle;
	dialogData.defaultName = defaultName;
	dialogData.done = false;

	g_main_context_invoke(g_main_loop_get_context(ofGstUtils::getGstMainLoop()), &file_dialog_gtk, &dialogData);
	if(!dialogData.done){
		dialogData.mutex.lock();
		dialogData.condition.wait(dialogData.mutex);
	}
	return dialogData.results;
}

#endif

#ifdef TARGET_ANDROID
#include "ofxAndroidUtils.h"
#endif

#ifdef TARGET_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

//------------------------------------------------------------------------------
ofFileDialogResult::ofFileDialogResult(){
	filePath = "";
	fileName = "";
	bSuccess = false;
}

//------------------------------------------------------------------------------
string ofFileDialogResult::getName(){
	return fileName;
}

//------------------------------------------------------------------------------
string ofFileDialogResult::getPath(){
	return filePath;
}


//------------------------------------------------------------------------------
void ofSystemAlertDialog(string errorMessage){
	#ifdef TARGET_WIN32
		// we need to convert error message to a wide char message.
		// first, figure out the length and allocate a wchar_t at that length + 1 (the +1 is for a terminating character)
		int length = strlen(errorMessage.c_str());
		wchar_t * widearray = new wchar_t[length+1];
		memset(widearray, 0, sizeof(wchar_t)*(length+1));
		// then, call mbstowcs:
		// http://www.cplusplus.com/reference/clibrary/cstdlib/mbstowcs/
		mbstowcs(widearray, errorMessage.c_str(), length);
		// launch the alert:
		MessageBoxW(nullptr, widearray, L"alert", MB_OK);
		// clear the allocated memory:
		delete widearray;
	#endif

	#ifdef TARGET_OSX
		@autoreleasepool {
			NSAlert* alertDialog = [[[NSAlert alloc] init] autorelease];
			alertDialog.messageText = [NSString stringWithUTF8String:errorMessage.c_str()];
			[alertDialog runModal];
			restoreAppWindowFocus();
		}
	#endif

	#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)
		initGTK();
		TextDialogData dialogData;
		dialogData.text = errorMessage;
		dialogData.done = false;
		g_main_context_invoke(g_main_loop_get_context(ofGstUtils::getGstMainLoop()), &alert_dialog_gtk, &dialogData);
		if(!dialogData.done){
			std::unique_lock<std::mutex> lock(dialogData.mutex);
			dialogData.condition.wait(lock);
		}
	#endif

	#ifdef TARGET_ANDROID
		ofxAndroidAlertBox(errorMessage);
	#endif

	#ifdef TARGET_EMSCRIPTEN
		emscripten_run_script((string("alert(")+errorMessage+");").c_str());
	#endif
}
#include <QApplication>
#include <QtWidgets>

#if defined( _WIN32 )
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include "mainwindow.h"

#if defined( _WIN32 )
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR argv, int argzc) {
	SetDllDirectoryA("./contentbuildergui/bin/");
#else
int main(int argc, char* argv[]) {
#endif // _WIN32
	int argc = 0;
	QApplication qApplication(argc, (char**)argv);
	MainWindow mainWindow;
	mainWindow.show();
	return qApplication.exec();
}

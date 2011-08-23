#include "Log.h"
#include "Platform.h"
#include "StringUtils.h"
#include "UpdateScript.h"
#include "UpdaterOptions.h"

#include "tinythread.h"

#if defined(PLATFORM_LINUX) and defined(ENABLE_GTK)
  #include "UpdateDialogGtk.h"
#endif

#include <iostream>

void runWithUi(int argc, char** argv, UpdateInstaller* installer);

void runUpdaterThread(void* arg)
{
	try
	{
		UpdateInstaller* installer = static_cast<UpdateInstaller*>(arg);
		installer->run();
	}
	catch (const std::exception& ex)
	{
		LOG(Error,"Unexpected exception " + std::string(ex.what()));
	}
}

int main(int argc, char** argv)
{
	UpdaterOptions options;
	options.parse(argc,argv);

	UpdateInstaller installer;
	UpdateScript script;

	if (!options.script.empty())
	{
		script.parse(options.script);
	}

	LOG(Info,"started updater. install-dir: " + options.installDir
	         + ", package-dir: " + options.packageDir
	         + ", wait-pid: " + intToStr(options.waitPid)
	         + ", script-path: " + options.script
	         + ", mode: " + intToStr(options.mode));

	installer.setMode(options.mode);
	installer.setInstallDir(options.installDir);
	installer.setPackageDir(options.packageDir);
	installer.setScript(&script);
	installer.setWaitPid(options.waitPid);

	if (options.mode == UpdateInstaller::Main)
	{
		runWithUi(argc,argv,&installer);
	}
	else
	{
		installer.run();
	}

	return 0;
}

#ifdef PLATFORM_LINUX
void runWithUi(int argc, char** argv, UpdateInstaller* installer)
{
#ifdef ENABLE_GTK
	LOG(Info,"setting up GTK UI");
	UpdateDialogGtk dialog;
	installer->setObserver(&dialog);
	dialog.init(argc,argv);
	tthread::thread updaterThread(runUpdaterThread,installer);
	dialog.exec();
	updaterThread.join();

	if (dialog.restartApp())
	{
		LOG(Info,"Restarting app after install");
		installer->restartMainApp();
	}
#else
	// no UI available - do a silent install
	installer->run();
	installer->restartMainApp();
#endif
}
#endif

#ifdef PLATFORM_MAC
void runWithUi(int argc, char** argv, UpdateInstaller* installer)
{
	// TODO - Cocoa UI
	installer->run();
}
#endif

#ifdef PLATFORM_WINDOWS
void runWithUi(int argc, char** argv, UpdateInstaller* installer)
{
	// TODO - Windows UI
	installer->run();
}
#endif

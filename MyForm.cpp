#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;



[STAThreadAttribute]

int Main()
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	genprj::MyForm form;
	Application::Run(% form);
	return 0;
}
//array<String^>^ args
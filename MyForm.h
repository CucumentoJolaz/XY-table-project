#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <string>
#include <vcclr.h>
#include <ctime>
#include <msclr\marshal_cppstd.h>
#include <fstream>
#include <sstream>


using namespace cv;
using namespace std;
using namespace System::Runtime::InteropServices;

CvCapture* capture = 0;
//IplImage* frame = 0;

//
// функция-обработчик ползунка - 
// перематывает на нужный кадр
HANDLE hComm;
DCB dcbSerialParams = { 0 }; // Initializing DCB structure
COMMTIMEOUTS timeouts = { 0 };
DWORD dNoOFBytestoWrite;         // No of bytes to write into the port
DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port
bool Status;
VideoCapture cap;
ofstream myfile;
ofstream myfile_steps;
int numOfRot = 0;
string stepLog = "stepLog.txt";
int XLeftBord = -24000;
int XRightBord = 24000;
int YLeftBord = -24000;
int YRightBord = 24000;


// Функция которая отправляет байты в плату для движения двигателей, 
//ей нужно отправить только в какую сторону, и на какой расстояние нужно двигуть платформу
//третий параметр градуирован на 1 мкм, т.е. если LenMov = 10, то платформа двигается на 10 мкм

namespace genprj {


	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;
	using namespace System::Timers;
	using namespace System::Threading;

	/// <summary>
	/// Сводка для MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
		System::Timers::Timer^ TimerID;
	private: System::Windows::Forms::Button^ folderMenuItem;
	public: System::Windows::Forms::Button^ port_test;
	private: System::Windows::Forms::Label^ label1;
	private: System::Windows::Forms::Timer^ timer1;
	private: System::Windows::Forms::Timer^ timer2;
	private: System::ComponentModel::BackgroundWorker^ backgroundWorker1;
	private: System::ComponentModel::BackgroundWorker^ backgroundWorker2;
	private: System::Windows::Forms::Button^ buttonUp;

	private: System::Windows::Forms::Button^ buttonDown;
	private: System::Windows::Forms::Button^ buttonRight;
	private: System::Windows::Forms::Button^ buttonLeft;



	public:
	private:

			 System::String^ folderName;
			 System::String^ errorText = "";
			 
	public:
		int Rotate(unsigned int XorY, unsigned int Dir, int LenMov) //1 пар не работает, 2 пар - направление двигателя, 1 - вправо, 0 влево, 3 пар - кол-во шагов на 1.8 градуса
		{

			if (LenMov == 0)
				return 0;

			char ByteToSend;

			if (XorY) // истина означает движение по X, ложь по Y
				if (Dir) // истина -  вверх/вправо
					ByteToSend |= 'c';
				else
					ByteToSend |= 'v';
			else
				if (Dir) // истина -  вверх/вправо
					ByteToSend |= 'b';
				else
					ByteToSend |= 'n';
			char lpBuffer[1] = { 0 };
			lpBuffer[0] = ByteToSend;

			dNoOFBytestoWrite = sizeof(lpBuffer);
			myfile << "Rotate started (3)" << XorY << Dir << LenMov << "\r\n";
			myfile << "num of rotation is " << numOfRot << "\r\n";
			myfile.close();
			myfile.open("log.txt", 'a');
			numOfRot++;

			int numOfBytes = LenMov;
			while(numOfBytes)
			{
				Status = WriteFile(hComm,        // Handle to the Serial port
					lpBuffer,     // Data to be written to the port
					dNoOFBytestoWrite,  //No of bytes to write
					&dNoOfBytesWritten, //Bytes written
					NULL);
				Sleep(1);
				numOfBytes--;
				/*
				std::time_t result = std::time(nullptr);

				if (Status)
				{

					myfile <<  std::asctime(std::localtime(&result)) << "Byte send" << "\r\n";
				}
				else
				{
					myfile <<  std::asctime(std::localtime(&result)) << "Byte wasn't send" << "\r\n";
					myfile << "Error number" << GetLastError() << "\r\n";
				}*/
			}
			
		}
		//int VideoStart() {}
		void DrawCVImage(System::Windows::Forms::Control^ control, cv::Mat& colorImage)
		{
			System::Drawing::Graphics^ graphics = control->CreateGraphics();
			System::IntPtr ptr(colorImage.ptr());
			System::Drawing::Bitmap^ b = gcnew System::Drawing::Bitmap(colorImage.cols, colorImage.rows, colorImage.step, System::Drawing::Imaging::PixelFormat::Format24bppRgb, ptr);
			System::Drawing::RectangleF rect(0, 0, control->Width, control->Height);
			graphics->DrawImage(b, rect);

			delete graphics;
		}

		void ReadCOM()
		{
			DWORD iSize;
			char sReceivedChar;
			while (true)
			{
				ReadFile(hComm, &sReceivedChar, 1, &iSize, 0);// // получаем 1 байт
				if (iSize > 0) //// если что-то принято, выводим
					this->LogBox->Text += sReceivedChar;

			}
		};

		int stepCheck(unsigned int XorY, unsigned int Dir, int LenMov)
		{
			ifstream fileStepLog;
			fileStepLog.open(stepLog);
			if (fileStepLog.is_open()) {
				fileStepLog.seekg(-1, ios_base::end);                // go to one spot before the EOF

				bool keepLooping = true;
				while (keepLooping) {
					char ch;
					fileStepLog.get(ch);                            // Get current byte's data

					if ((int)fileStepLog.tellg() <= 1) {             // If the data was at or before the 0th byte
						fileStepLog.seekg(0);                       // The first line is the last line
						keepLooping = false;                // So stop there
					}
					else if (ch == '\n') {                   // If the data was a newline
						keepLooping = false;                // Stop at the current position.
					}
					else {                                  // If the data was neither a newline nor at the 0 byte
						fileStepLog.seekg(-2, ios_base::cur);        // Move to the front of that data, then to the front of the data before it
					}
				}

				string lastLine;
				getline(fileStepLog, lastLine);                      // Read the current line
				cout << "Result: " << lastLine << '\n';     // Display it

				fileStepLog.close();
				auto pos = lastLine.find(";");

				if (pos != string::npos)
				{
					float X = std::stof(lastLine.substr(0, pos));
					float Y = std::stof(lastLine.substr(pos + 1));
				}

			}
		}
		int logStep()
		{

		}
		int firstTimerStart = 1;
		int BW1ShouldWork = 0;
		int BW2ShouldWork = 0;
		int timerStart = 0;
		int makeAShoot = 0;
		int MKSworking = 0;
		int XstpLen;
		int YstpLen;
		int XstpQnt;
		int YstpQnt;
		const int Xmov = 1;
		const int Ymov = 0;
		const int Rightmov = 1;
		const int Leftmov = 0;
		const int Upmov = 1;
		const int Downmov = 0;
		const int HungredSteps = 100;
		int BGW2sleepTime = 0;


		int Ystep = 1;
		int Xstep = 1;

	private: System::Windows::Forms::TextBox^ folderNameTextBox;


	private: System::Windows::Forms::FolderBrowserDialog^ folderBrowserDialog1;



	public:
		int numOfCapture = 1; // номер кадра

		MyForm(void)
		{
			myfile.open("log.txt");
			myfile_steps.open("log_steps.txt");
			InitializeComponent();
			TimerID = gcnew System::Timers::Timer();
			TimerID->Elapsed += gcnew System::Timers::ElapsedEventHandler(this, &MyForm::TimerID_Tick);

			//
			//TODO: добавьте код конструктора
			//
		}


	private: System::Windows::Forms::ComboBox^ comboBox3;
	private: System::Windows::Forms::OpenFileDialog^ openFileDialog1;


	public:
		int FlagCapture = 0;
	protected:
		/// <summary>
		/// Освободить все используемые ресурсы.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::TextBox^ ComPortNameText;
	private: System::Windows::Forms::Label^ ComPortName;
	private: System::Windows::Forms::Button^ UpdateTheConfig;
	private: System::Windows::Forms::TextBox^ LogBox;




	private: System::Windows::Forms::Label^ label3;
	private: System::Windows::Forms::Label^ label4;
	private: System::Windows::Forms::Label^ label5;
	private: System::Windows::Forms::Label^ label6;
	private: System::Windows::Forms::Button^ Begin_capture;

	private: System::Windows::Forms::Label^ label7;
	private: System::Windows::Forms::TextBox^ Y_step_quantity;
	private: System::Windows::Forms::TextBox^ X_step_quantity;
	private: System::Windows::Forms::TextBox^ X_step_value;
	private: System::Windows::Forms::TextBox^ Y_step_value;
	private: System::ComponentModel::IContainer^ components;

	protected:

	protected:



#pragma region Windows Form Designer generated code
		/// <summary>
		/// Требуемый метод для поддержки конструктора — не изменяйте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->ComPortNameText = (gcnew System::Windows::Forms::TextBox());
			this->ComPortName = (gcnew System::Windows::Forms::Label());
			this->UpdateTheConfig = (gcnew System::Windows::Forms::Button());
			this->LogBox = (gcnew System::Windows::Forms::TextBox());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->Begin_capture = (gcnew System::Windows::Forms::Button());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->Y_step_quantity = (gcnew System::Windows::Forms::TextBox());
			this->X_step_quantity = (gcnew System::Windows::Forms::TextBox());
			this->X_step_value = (gcnew System::Windows::Forms::TextBox());
			this->Y_step_value = (gcnew System::Windows::Forms::TextBox());
			this->comboBox3 = (gcnew System::Windows::Forms::ComboBox());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->folderNameTextBox = (gcnew System::Windows::Forms::TextBox());
			this->folderBrowserDialog1 = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->folderMenuItem = (gcnew System::Windows::Forms::Button());
			this->port_test = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->timer2 = (gcnew System::Windows::Forms::Timer(this->components));
			this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
			this->backgroundWorker2 = (gcnew System::ComponentModel::BackgroundWorker());
			this->buttonUp = (gcnew System::Windows::Forms::Button());
			this->buttonDown = (gcnew System::Windows::Forms::Button());
			this->buttonRight = (gcnew System::Windows::Forms::Button());
			this->buttonLeft = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(211, 65);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(148, 23);
			this->button1->TabIndex = 1;
			this->button1->Text = L"Start";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			// 
			// ComPortNameText
			// 
			this->ComPortNameText->Location = System::Drawing::Point(44, 39);
			this->ComPortNameText->Name = L"ComPortNameText";
			this->ComPortNameText->Size = System::Drawing::Size(113, 20);
			this->ComPortNameText->TabIndex = 3;
			this->ComPortNameText->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox2_TextChanged);
			// 
			// ComPortName
			// 
			this->ComPortName->AutoSize = true;
			this->ComPortName->Location = System::Drawing::Point(41, 23);
			this->ComPortName->Name = L"ComPortName";
			this->ComPortName->Size = System::Drawing::Size(116, 13);
			this->ComPortName->TabIndex = 4;
			this->ComPortName->Text = L"Название COM порта";
			this->ComPortName->Click += gcnew System::EventHandler(this, &MyForm::label1_Click);
			// 
			// UpdateTheConfig
			// 
			this->UpdateTheConfig->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->UpdateTheConfig->Location = System::Drawing::Point(32, 65);
			this->UpdateTheConfig->Name = L"UpdateTheConfig";
			this->UpdateTheConfig->Size = System::Drawing::Size(153, 23);
			this->UpdateTheConfig->TabIndex = 5;
			this->UpdateTheConfig->Text = L"Open the COM port";
			this->UpdateTheConfig->UseVisualStyleBackColor = true;
			this->UpdateTheConfig->Click += gcnew System::EventHandler(this, &MyForm::UpdateTheConfig_Click);
			// 
			// LogBox
			// 
			this->LogBox->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->LogBox->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->LogBox->Location = System::Drawing::Point(0, 279);
			this->LogBox->Multiline = true;
			this->LogBox->Name = L"LogBox";
			this->LogBox->ReadOnly = true;
			this->LogBox->RightToLeft = System::Windows::Forms::RightToLeft::No;
			this->LogBox->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->LogBox->Size = System::Drawing::Size(728, 471);
			this->LogBox->TabIndex = 6;
			this->LogBox->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox3_TextChanged);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(41, 123);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(132, 13);
			this->label3->TabIndex = 16;
			this->label3->Text = L"Величина шага по оси X ";
			this->label3->Click += gcnew System::EventHandler(this, &MyForm::label3_Click);
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(46, 172);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(129, 13);
			this->label4->TabIndex = 17;
			this->label4->Text = L"Величина шага по оси Y";
			this->label4->Click += gcnew System::EventHandler(this, &MyForm::label4_Click);
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(199, 124);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(146, 13);
			this->label5->TabIndex = 18;
			this->label5->Text = L"Количество шагов по оси X";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(199, 172);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(146, 13);
			this->label6->TabIndex = 19;
			this->label6->Text = L"Количество шагов по оси Y";
			this->label6->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// Begin_capture
			// 
			this->Begin_capture->Enabled = false;
			this->Begin_capture->Location = System::Drawing::Point(580, 234);
			this->Begin_capture->Name = L"Begin_capture";
			this->Begin_capture->Size = System::Drawing::Size(123, 52);
			this->Begin_capture->TabIndex = 20;
			this->Begin_capture->Text = L"Запустить съёмку";
			this->Begin_capture->UseVisualStyleBackColor = true;
			this->Begin_capture->Click += gcnew System::EventHandler(this, &MyForm::Begin_capture_Click);
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(21, 235);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(150, 13);
			this->label7->TabIndex = 22;
			this->label7->Text = L"Папка для сохранения фото";
			// 
			// Y_step_quantity
			// 
			this->Y_step_quantity->Location = System::Drawing::Point(202, 189);
			this->Y_step_quantity->Name = L"Y_step_quantity";
			this->Y_step_quantity->Size = System::Drawing::Size(100, 20);
			this->Y_step_quantity->TabIndex = 23;
			this->Y_step_quantity->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox1_TextChanged_1);
			// 
			// X_step_quantity
			// 
			this->X_step_quantity->Location = System::Drawing::Point(202, 139);
			this->X_step_quantity->Name = L"X_step_quantity";
			this->X_step_quantity->Size = System::Drawing::Size(100, 20);
			this->X_step_quantity->TabIndex = 24;
			this->X_step_quantity->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox1_TextChanged_2);
			// 
			// X_step_value
			// 
			this->X_step_value->Location = System::Drawing::Point(49, 140);
			this->X_step_value->Name = L"X_step_value";
			this->X_step_value->Size = System::Drawing::Size(100, 20);
			this->X_step_value->TabIndex = 25;
			this->X_step_value->TextChanged += gcnew System::EventHandler(this, &MyForm::X_step_value_TextChanged);
			// 
			// Y_step_value
			// 
			this->Y_step_value->Location = System::Drawing::Point(49, 189);
			this->Y_step_value->Name = L"Y_step_value";
			this->Y_step_value->Size = System::Drawing::Size(100, 20);
			this->Y_step_value->TabIndex = 26;
			this->Y_step_value->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox3_TextChanged_2);
			// 
			// comboBox3
			// 
			this->comboBox3->FormattingEnabled = true;
			this->comboBox3->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Capture From Camera", L"Capture From File" });
			this->comboBox3->Location = System::Drawing::Point(211, 38);
			this->comboBox3->Name = L"comboBox3";
			this->comboBox3->Size = System::Drawing::Size(125, 21);
			this->comboBox3->TabIndex = 28;
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			// 
			// folderNameTextBox
			// 
			this->folderNameTextBox->Location = System::Drawing::Point(24, 251);
			this->folderNameTextBox->Name = L"folderNameTextBox";
			this->folderNameTextBox->Size = System::Drawing::Size(304, 20);
			this->folderNameTextBox->TabIndex = 21;
			// 
			// folderMenuItem
			// 
			this->folderMenuItem->Location = System::Drawing::Point(334, 229);
			this->folderMenuItem->Name = L"folderMenuItem";
			this->folderMenuItem->Size = System::Drawing::Size(100, 45);
			this->folderMenuItem->TabIndex = 29;
			this->folderMenuItem->Text = L"Выбрать папку для сохранения";
			this->folderMenuItem->UseVisualStyleBackColor = true;
			this->folderMenuItem->Click += gcnew System::EventHandler(this, &MyForm::folderMenuItem_Click);
			// 
			// port_test
			// 
			this->port_test->Enabled = false;
			this->port_test->Location = System::Drawing::Point(518, 39);
			this->port_test->Name = L"port_test";
			this->port_test->Size = System::Drawing::Size(75, 23);
			this->port_test->TabIndex = 30;
			this->port_test->Text = L"Тест порта";
			this->port_test->UseMnemonic = false;
			this->port_test->UseVisualStyleBackColor = true;
			this->port_test->UseWaitCursor = true;
			this->port_test->Click += gcnew System::EventHandler(this, &MyForm::Port_test_Click);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(12, 100);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(184, 13);
			this->label1->TabIndex = 31;
			this->label1->Text = L"Единица \"Величины шага\" = 5 мкм";
			this->label1->Click += gcnew System::EventHandler(this, &MyForm::Label1_Click_2);
			// 
			// timer1
			// 
			this->timer1->Tick += gcnew System::EventHandler(this, &MyForm::timer1_Tick);
			// 
			// backgroundWorker1
			// 
			this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::backgroundWorker1_DoWork);
			this->backgroundWorker1->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::backgroundWorker1_RunWorkerCompleted);
			// 
			// backgroundWorker2
			// 
			this->backgroundWorker2->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::backgroundWorker2_DoWork);
			this->backgroundWorker2->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::backgroundWorker2_RunWorkerCompleted);
			// 
			// buttonUp
			// 
			this->buttonUp->Location = System::Drawing::Point(580, 90);
			this->buttonUp->Name = L"buttonUp";
			this->buttonUp->Size = System::Drawing::Size(41, 23);
			this->buttonUp->TabIndex = 32;
			this->buttonUp->Text = L"△";
			this->buttonUp->UseVisualStyleBackColor = true;
			this->buttonUp->Click += gcnew System::EventHandler(this, &MyForm::ButtonUp_Click);
			// 
			// buttonDown
			// 
			this->buttonDown->Location = System::Drawing::Point(580, 162);
			this->buttonDown->Name = L"buttonDown";
			this->buttonDown->Size = System::Drawing::Size(41, 23);
			this->buttonDown->TabIndex = 33;
			this->buttonDown->Text = L"▽";
			this->buttonDown->UseVisualStyleBackColor = true;
			this->buttonDown->Click += gcnew System::EventHandler(this, &MyForm::ButtonDown_Click);
			// 
			// buttonRight
			// 
			this->buttonRight->Location = System::Drawing::Point(638, 123);
			this->buttonRight->Name = L"buttonRight";
			this->buttonRight->Size = System::Drawing::Size(41, 23);
			this->buttonRight->TabIndex = 34;
			this->buttonRight->Text = L"ᐅ";
			this->buttonRight->UseVisualStyleBackColor = true;
			this->buttonRight->Click += gcnew System::EventHandler(this, &MyForm::ButtonRight_Click);
			// 
			// buttonLeft
			// 
			this->buttonLeft->Location = System::Drawing::Point(518, 123);
			this->buttonLeft->Name = L"buttonLeft";
			this->buttonLeft->Size = System::Drawing::Size(41, 23);
			this->buttonLeft->TabIndex = 35;
			this->buttonLeft->Text = L"ᐊ";
			this->buttonLeft->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->buttonLeft->UseVisualStyleBackColor = true;
			this->buttonLeft->Click += gcnew System::EventHandler(this, &MyForm::ButtonLeft_Click);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(728, 750);
			this->Controls->Add(this->buttonLeft);
			this->Controls->Add(this->buttonRight);
			this->Controls->Add(this->buttonDown);
			this->Controls->Add(this->buttonUp);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->port_test);
			this->Controls->Add(this->folderMenuItem);
			this->Controls->Add(this->comboBox3);
			this->Controls->Add(this->Y_step_value);
			this->Controls->Add(this->X_step_value);
			this->Controls->Add(this->X_step_quantity);
			this->Controls->Add(this->Y_step_quantity);
			this->Controls->Add(this->label7);
			this->Controls->Add(this->folderNameTextBox);
			this->Controls->Add(this->Begin_capture);
			this->Controls->Add(this->label6);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->LogBox);
			this->Controls->Add(this->UpdateTheConfig);
			this->Controls->Add(this->ComPortName);
			this->Controls->Add(this->ComPortNameText);
			this->Controls->Add(this->button1);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void listBox1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void listBox1_SelectedIndexChanged_1(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
		if (comboBox3->Text == "")
		{
			MessageBox::Show(this, "Select Capture Method", "Error!!!");
		}
		if (button1->Text == "Start")
		{
			namedWindow("A_good_name", WINDOW_AUTOSIZE); //create a window called "MyVideo"

			if (comboBox3->Text == "Capture From Camera")
			{
				cap.open(0);
				if (!cap.isOpened()) {
					this->LogBox->Text += "ERROR! Unable to open camera" + "\r\n";
				}
				//trackBar1->Minimum = 0;
				//trackBar1->Maximum = 0;
				button1->Text = "Stop";
				TimerID->Start();
				this->Begin_capture->Enabled = true;
			}
			else if (comboBox3->Text == "Capture From File")
			{
				//Uncomment the following line if you want to start the video in the middle
				//cap.set(CAP_PROP_POS_MSEC, 300); 
				//get the frames rate of the video
				OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;

				//String window_name = "My First Video";
				openFileDialog1->Filter = "AVI files (*.avi)|*.txt|All files (*.*)|*.*";
				openFileDialog1->FilterIndex = 2;
				openFileDialog1->RestoreDirectory = true;
				openFileDialog1->FileName = "";
				if (openFileDialog1->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
				{
					char* fileName = (char*)Marshal::StringToHGlobalAnsi(openFileDialog1->FileName).ToPointer();
					cap.open(fileName);
					//trackBar1->Minimum = 0;
					//trackBar1->Maximum = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
					button1->Text = "Stop";
					double fps = cap.get(5);

					TimerID->Interval = 1 / fps * 1000;
					//this->LogBox->Text += TimerID->Interval + "\r\n";
					TimerID->Enabled = true;
					TimerID->Start();

					if (cap.isOpened() == false)
					{
						this->LogBox->Text += "Cannot open the video file" + "\r\n";
						cin.get(); //wait for any key press
					}
					this->Begin_capture->Enabled = true;
					this->LogBox->Text += "Frames per seconds : " + fps + "\r\n";
				}
				else
					this->LogBox->Text += "openFileDialog1->ShowDialog() have not been open " + "\r\n";
			}
		}
		else if (button1->Text == "Stop")
		{
			button1->Text = "Start";
			TimerID->Stop();
			this->Begin_capture->Enabled = false;
		}
	}

	private: System::Void label1_Click(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox2_TextChanged(System::Object^ sender, System::EventArgs^ e) { // форма для имени COM порта

	}
	private: System::Void label2_Click(System::Object^ sender, System::EventArgs^ e) {

	}
	private: System::Void textBox3_TextChanged(System::Object^ sender, System::EventArgs^ e) { // основной лог
	}
	private: System::Void UpdateTheConfig_Click(System::Object^ sender, System::EventArgs^ e) { // кнопка для обновления конфигурации COM порта
		pin_ptr<const wchar_t> wch = PtrToStringChars(this->ComPortNameText->Text);
		hComm = CreateFile(wch,                //port name
			GENERIC_READ | GENERIC_WRITE, //Read/Write
			0,                            // No Sharing
			NULL,                         // No Security
			OPEN_EXISTING,// Open existing port only
			0,            // Non Overlapped I/O
			NULL);        // Null for Comm Devices
		// имя файла задаётся первым параметром
		if (this->UpdateTheConfig->Text == L"Open the COM port")
		{
			if (hComm == INVALID_HANDLE_VALUE)
				this->LogBox->Text += "Error in opening serial port" + "\r\n";
			else
			{
				this->LogBox->Text += "Opening serial port successful" + "\r\n";
				this->Begin_capture->Enabled = true;
				this->port_test->Enabled = true;
				dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
				Status = GetCommState(hComm, &dcbSerialParams);

				if (!Status)
				{
					this->LogBox->Text += "getting state error\n";
				}

				dcbSerialParams.BaudRate = CBR_9600;  // Setting BaudRate = 9600
				dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
				dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
				dcbSerialParams.Parity = NOPARITY;  // Setting Parity = None

				timeouts.ReadIntervalTimeout = 50; // in milliseconds
				timeouts.ReadTotalTimeoutConstant = 50; // in milliseconds
				timeouts.ReadTotalTimeoutMultiplier = 10; // in milliseconds
				timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
				timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

				if (!SetCommState(hComm, &dcbSerialParams))
				{
					this->LogBox->Text += "error setting serial port state\n";
				}


				this->UpdateTheConfig->Text = L"Close the COM port";
				//Rotate(1, 1, 300);
			}
		}
		else
		{
			if (CloseHandle(hComm))//Closing the Serial Port
				this->LogBox->Text += "Closing serial port successful" + "\r\n";
			this->UpdateTheConfig->Text = L"Open the COM port";
			this->Begin_capture->Enabled = false;
			this->port_test->Enabled = false;
		}
	}
	private: System::Void label1_Click_1(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox4_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox3_TextChanged_1(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void label3_Click(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void label4_Click(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void Begin_capture_Click(System::Object^ sender, System::EventArgs^ e) {

		if (this->Begin_capture->Text == L"Запустить съёмку")
		{
			
			XstpLen = Int32::Parse(X_step_value->Text);
			YstpLen = Int32::Parse(Y_step_value->Text);
			XstpQnt = Int32::Parse(X_step_quantity->Text);
			YstpQnt = Int32::Parse(Y_step_quantity->Text);
			this->LogBox->Text += "XstpQnt = " + XstpQnt + " YstpQnt = " + YstpQnt + " XstpLen = " + XstpLen + " XstpLen = " + XstpLen + "\r\n";

			
			

			if (XstpQnt == 0 || YstpQnt == 0 || XstpLen == 0 || YstpLen == 0)
				errorText += "Значения шагов для двигателя не верны" + "\r\n";
				//this->LogBox->Text += "Значения шагов для двигателя не верны" + "\r\n";
			else {
				this->Begin_capture->Text = L"Прервать съёмку";
				//СЧЁТ НАЧИНАЕМ С ЕДИНИЦЫ!!!! НЕ С НУЛЯ
				this->Begin_capture->Enabled = false;
				this->LogBox->Text += "The capture has been started" + "\r\n";
				Ystep = 1;
				Xstep = 1;
				//this->LogBox->Text += "makeAShot Check" + "\r\n";
				//this->LogBox->Text += fileName + "\r\n";
				//this->LogBox->Text += "Current filename is " + fileName + "\r\n";
				//this->LogBox->Text += "Ystep = " + Ystep + "  Xstep = " + Xstep + "\r\n";

				for (; Ystep <= YstpQnt; Ystep++) 
				{
					for (; Xstep <= XstpQnt; Xstep++)
					{
						Mat frame;

						bool bSuccess = cap.read(frame); // read a new frame from video 
						if (bSuccess == false)
						{

							this->LogBox->Text += "Found the end of the video" + "\r\n";
						}
						else {
							myfile_steps << "Ystep = " << Ystep << "  Xstep = " << Xstep << endl;
							numOfCapture = (Ystep - 1) * XstpQnt + Xstep;

							System::String^ fileName = this->folderNameTextBox->Text + "//" + numOfCapture + ".jpg";

							std::string  fileNameUnmanaged = msclr::interop::marshal_as<std::string>(fileName);
							imwrite(fileNameUnmanaged, frame);
							//this->LogBox->Text += numOfCapture + " saved" + "\n";
							myfile_steps << numOfCapture << " saved" << endl;

							if (!(Ystep % 2))
							{
								Rotate(Xmov, Rightmov, XstpLen);
							}
							else
							{
								Rotate(Xmov, Leftmov, XstpLen);
							}
							Sleep(2000);
						}
					}
					Rotate(Ymov, Leftmov, YstpLen);
					Xstep = 1;
					Sleep(2000);
				}
				


				



				if (numOfCapture >= XstpQnt * YstpQnt)
				{
					timer1->Stop();
					this->LogBox->Text += "таймер1 остановлен окончательно" + "\r\n";
				}

				//timerStart = 1;

				//TimerID->Start();
				//this->LogBox->Text += "BW2ShouldWork = " + BW2ShouldWork + "\r\n";
			}
		}
		else if (this->Begin_capture->Text == L"Прервать съёмку")
		{

		}
		
	}
	private: System::Void textBox1_TextChanged_1(System::Object^ sender, System::EventArgs^ e) {
	}

	private: System::Void textBox1_TextChanged_2(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox3_TextChanged_2(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void X_step_value_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void MyForm_Load(System::Object^ sender, System::EventArgs^ e) {
	}


	private: System::Void openFileDialog1_FileOk(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e) {
	}

	private: System::Void MyForm::TimerID_Tick(System::Object^ sender, System::Timers::ElapsedEventArgs^ e)
	{

		Mat frame;

		bool bSuccess = cap.read(frame); // read a new frame from video 
		if (bSuccess == false)
		{

			this->LogBox->Text += "Found the end of the video" + "\r\n";
			//this->Begin_capture->Enabled = true; // обратное включение кнопки после съёмки
			TimerID->Stop();
		}
		else
		{
			imshow("A_good_name", frame);
			if (timerStart)
			{
				timer1->Interval = 2996;
				timer1->Start();
				this->LogBox->Text += "таймер1 запущен" + "\r\n";
				timerStart = 0;

			}
			if (makeAShoot)
			{


				this->LogBox->Text += "makeAShot Check" + "\r\n";
				//this->LogBox->Text += fileName + "\r\n";
				//this->LogBox->Text += "Current filename is " + fileName + "\r\n";
				//this->LogBox->Text += "Ystep = " + Ystep + "  Xstep = " + Xstep + "\r\n";

				myfile_steps << "Ystep = " << Ystep << "  Xstep = " << Xstep << endl;
				numOfCapture = (Ystep - 1) * XstpQnt + Xstep;

				System::String^ fileName = this->folderNameTextBox->Text + "//" + numOfCapture + ".jpg";

				std::string  fileNameUnmanaged = msclr::interop::marshal_as<std::string>(fileName);
				imwrite(fileNameUnmanaged, frame);
				//this->LogBox->Text += numOfCapture + " saved" + "\n";

				myfile_steps << numOfCapture << " saved" << endl;


				//this->Begin_capture->Enabled = true;

				++Xstep;
				if (Xstep > XstpQnt) // активация переменных ответственных за цикл произвотился кнопкой начала съёмки				                                              
				{// это функция Begin_capture_Click
					//Sleep(1000);
					Rotate(Ymov, Leftmov, YstpLen);
					Ystep++;
					Xstep = 1;
				}
				else {
					if (!(Ystep % 2))
					{
						Rotate(Xmov, Rightmov, XstpLen);
					}
					else
					{
						Rotate(Xmov, Leftmov, XstpLen);
					}
				}



				if (numOfCapture >= XstpQnt * YstpQnt)
				{
					timer1->Stop();
					this->LogBox->Text += "таймер1 остановлен окончательно" + "\r\n";
				}


				makeAShoot = 0;



			}
		}
		//Breaking the while loop at the end of the video
		if (waitKey(10) == 27)
		{
			this->LogBox->Text += "Esc key is pressed by user. Stopping the video" + "\r\n";
		}
	}
	private:  void backgroundWorker1_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {

		Sleep(1000);
		/*
		ParamsToPass YstpQnt = (ParamsToPass)e->YstpQntS;
		ParamsToPass XstpQnt = (ParamsToPass)e->YstpQntS;
		ParamsToPass YstpLen = (ParamsToPass)e->YstpQntS;
		ParamsToPass XstpLen = (ParamsToPass)e->YstpQntS;
		object[] parameters = e.Argument as object[];

		for (int Ystep = 1; Ystep <= e->YstpQntS; Ystep++)
		{
			for (int Xstep = 1; Xstep <= XstpQnt; Xstep++)
			{


			}*/
	}
	private:  void backgroundWorker2_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
		Sleep(2000);

	}
	private: void backgroundWorker1_ProgressChanged(Object^ /*sender*/, ProgressChangedEventArgs^ e)
	{
	}
	private: System::Void backgroundWorker1_RunWorkerCompleted(Object^ /*sender*/, RunWorkerCompletedEventArgs^ e)
	{
		makeAShoot = 1;
		//this->LogBox->Text += "backgroundWorker1_RunWorkerCompleted" + "\r\n";

		//myfile << "backgroundWorker1_RunWorkerCompleted" << " BW1ShouldWork = " << BW1ShouldWork << " BW2ShouldWork = " << BW2ShouldWork << " makeAShoot = " << makeAShoot << endl;
		//myfile << "Ystep = " << Ystep << "  Xstep = " << Xstep << "\r\n";

	}
	private: System::Void backgroundWorker2_RunWorkerCompleted(Object^ /*sender*/, RunWorkerCompletedEventArgs^ e)
	{
		BW1ShouldWork = 1;
		//this->LogBox->Text += "backgroundWorker2_RunWorkerCompleted" + "\r\n";
		//myfile << "backgroundWorker2_RunWorkerCompleted" << " BW1ShouldWork = " << BW1ShouldWork << " BW2ShouldWork = " << BW2ShouldWork << " makeAShoot = " << makeAShoot << endl;
		//myfile << "Ystep = " << Ystep << "  Xstep = " << Xstep << "\r\n";

	}


	private: System::Void folderMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
		{

			// Show the FolderBrowserDialog.
			System::Windows::Forms::DialogResult result = folderBrowserDialog1->ShowDialog();
			if (result == System::Windows::Forms::DialogResult::OK)
			{
				if (folderBrowserDialog1->SelectedPath != "")
				{
					folderName = folderBrowserDialog1->SelectedPath;
				}
				this->folderNameTextBox->Text = folderName;

			}
		}

	};
	private: System::Void Port_test_Click(System::Object^ sender, System::EventArgs^ e) {
		Rotate(1, 1, 1);
	}
private: System::Void Label1_Click_2(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void timer1_Tick(System::Object^ sender, System::EventArgs^ e) {
	makeAShoot = 1;
	System::DateTime^ now = System::DateTime::Now;
	std::time_t result = std::time(nullptr);
	//timerCounter++;
	myfile_steps << std::asctime(std::localtime(&result)) << " Timer ticked "  << endl;

	//timer1->Stop();
	

}
private: System::Void ButtonUp_Click(System::Object^ sender, System::EventArgs^ e) {
	Rotate(Ymov, Upmov, HungredSteps);
}
private: System::Void ButtonDown_Click(System::Object^ sender, System::EventArgs^ e) {
	Rotate(Ymov, Downmov, HungredSteps);
}
private: System::Void Button4_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void ButtonLeft_Click(System::Object^ sender, System::EventArgs^ e) {
	Rotate(Xmov, Leftmov, HungredSteps);
}
private: System::Void ButtonRight_Click(System::Object^ sender, System::EventArgs^ e) {
	Rotate(Xmov, Rightmov, HungredSteps);
}
};

}


///////////////////////////////////////////////////////////////////////////
// Window.cpp - C++\CLI implementation of WPF Application                //
//          - Demo for CSE 687 Project #4                                //
// ver 2.0                                                               //
// Source:Jim Fawcett, CSE687 - Object Oriented Design, Spring 2015      //
// Author: Qi Tan qtan100@syr.edu                                        //
///////////////////////////////////////////////////////////////////////////
/*
*  To run as a Windows Application:
*  - Set Project Properties > Linker > System > Subsystem to Windows
*  - Comment out int main(...) at bottom
*  - Uncomment int _stdcall WinMain() at bottom
*  To run as a Console Application:
*  - Set Project Properties > Linker > System > Subsytem to Console
*  - Uncomment int main(...) at bottom
*  - Comment out int _stdcall WinMain() at bottom
*/
#include "Window.h"
using namespace CppCliWindows;

WPFCppCliDemo::WPFCppCliDemo()
{
  // set up channel
  ObjectFactory* pObjFact = new ObjectFactory;
  pSendr_ = pObjFact->createSendr();
  pRecvr_ = pObjFact->createRecvr();
  pChann_ = pObjFact->createMockChannel(pSendr_, pRecvr_);
  pChann_->start();
  delete pObjFact;
  // client's receive thread
  recvThread = gcnew Thread(gcnew ThreadStart(this, &WPFCppCliDemo::getMessage));
  recvThread->Start();
  // set event handlers
  this->Loaded += 
    gcnew System::Windows::RoutedEventHandler(this, &WPFCppCliDemo::OnLoaded);
  this->Closing += 
    gcnew CancelEventHandler(this, &WPFCppCliDemo::Unloading);
  hSendButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::uploadFile);
  //hClearButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::clear);
  hSend->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::sendMessage);
  hClear->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::clear);
  hFolderBrowseButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::browseForFolder);
  hShowItemsButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::getItemsFromList);
  hGetFolder->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::getFolder);
  hDownloadWithD ->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::downloadWith);
  hDownloadWithoutD->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::downloadWithout);
  // set Window properties

  this->Title = "WPF C++/CLI Demo";
  this->Width = 800;
  this->Height = 600;

  // attach dock panel to Window

  this->Content = hDockPanel;
  hDockPanel->Children->Add(hStatusBar);
  hDockPanel->SetDock(hStatusBar, Dock::Bottom);
  hDockPanel->Children->Add(hGrid);

  // setup Window controls and views

  setUpTabControl();
  setUpStatusBar();
  setUpSendMessageView();
  setUpFileListView();
  setUpConnectionView();
}

WPFCppCliDemo::~WPFCppCliDemo()
{
  delete pChann_;
  delete pSendr_;
  delete pRecvr_;
}

void WPFCppCliDemo::setUpStatusBar()
{
  hStatusBar->Items->Add(hStatusBarItem);
 // hStatus->Text = "very important messages will appear here";
  //status->FontWeight = FontWeights::Bold;
  hStatusBarItem->Content = hStatus;
  hStatusBar->Padding = Thickness(10, 2, 10, 2);
}

void WPFCppCliDemo::setUpTabControl()
{
  hGrid->Children->Add(hTabControl);
  hSendMessageTab->Header = "Upload";
  hFileListTab->Header = "Dependency";
  hConnectTab->Header = "Download";
  hTabControl->Items->Add(hSendMessageTab);
  hTabControl->Items->Add(hFileListTab);
  hTabControl->Items->Add(hConnectTab);
}

void WPFCppCliDemo::setTextBlockProperties()
{
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hSendMessageGrid->RowDefinitions->Add(hRow1Def);
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hBorder1->Child = hTextBlock1;
  hTextBlock1->Padding = Thickness(15);
  hTextBlock1->Text = "";
  hTextBlock1->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
  hTextBlock1->FontWeight = FontWeights::Bold;
  hTextBlock1->FontSize = 16;
  hScrollViewer1->VerticalScrollBarVisibility = ScrollBarVisibility::Auto;
  hScrollViewer1->Content = hBorder1;
  hSendMessageGrid->SetRow(hScrollViewer1, 0);
  hSendMessageGrid->Children->Add(hScrollViewer1);
}

void WPFCppCliDemo::setButtonsProperties()
{
  RowDefinition^ hRow2Def = gcnew RowDefinition();
  hRow2Def->Height = GridLength(75);
  hSendMessageGrid->RowDefinitions->Add(hRow2Def);
  hSendButton->Content = "Send Message";
  Border^ hBorder2 = gcnew Border();
  hBorder2->Width = 120;
  hBorder2->Height = 30;
  hBorder2->BorderThickness = Thickness(1);
  hBorder2->BorderBrush = Brushes::Black;
  hClearButton->Content = "Clear";
  hBorder2->Child = hSendButton;
  Border^ hBorder3 = gcnew Border();
  hBorder3->Width = 120;
  hBorder3->Height = 30;
  hBorder3->BorderThickness = Thickness(1);
  hBorder3->BorderBrush = Brushes::Black;
  hBorder3->Child = hClearButton;
  hStackPanel1->Children->Add(hBorder2);
  TextBlock^ hSpacer = gcnew TextBlock();
  hSpacer->Width = 10;
  hStackPanel1->Children->Add(hSpacer);
  hStackPanel1->Children->Add(hBorder3);
  hStackPanel1->Orientation = Orientation::Horizontal;
  hStackPanel1->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  hSendMessageGrid->SetRow(hStackPanel1, 1);
  hSendMessageGrid->Children->Add(hStackPanel1);
}

void WPFCppCliDemo::setUpSendMessageView()
{
  Console::Write("\n  setting up sendMessage view");
  hSendMessageGrid->Margin = Thickness(20);
  hSendMessageTab->Content = hSendMessageGrid;

  setTextBlockProperties();
  setButtonsProperties();
}

std::string WPFCppCliDemo::toStdString(String^ pStr)
{
  std::string dst;
  for (int i = 0; i < pStr->Length; ++i)
    dst += (char)pStr[i];
  return dst;
}

void WPFCppCliDemo::sendMessage(Object^ obj, RoutedEventArgs^ args)
{
  msgText = "dependency";
  pSendr_->postMessage(toStdString(msgText));
  Console::Write("\n  sent message");
  hStatus->Text = "Sent message";
}

void WPFCppCliDemo::uploadFile(Object^ obj, RoutedEventArgs^ args)
{
	msgText = "file";
	pSendr_->postMessage(toStdString(msgText));
	Console::Write("\n  sent uploadFile");
	hStatus->Text = "Sent uploadFile";
}

void CppCliWindows::WPFCppCliDemo::getFolder(Object ^ obj, RoutedEventArgs ^ args)
{
	msgText = "fileList";
	pSendr_->postMessage(toStdString(msgText));
	Console::Write("\n  get folder list");
	hStatus->Text = "get folder list";
}

void CppCliWindows::WPFCppCliDemo::downloadWith(Object ^ obj, RoutedEventArgs ^ args)
{
	msgText = "downloadW";
	pSendr_->postMessage(toStdString(msgText));
	Console::Write("\n  download with dependency");
	hStatus->Text = "download with dependency";
}

void CppCliWindows::WPFCppCliDemo::downloadWithout(Object ^ obj, RoutedEventArgs ^ args)
{
	msgText = "download Without";
	pSendr_->postMessage(toStdString(msgText));
	Console::Write("\n  download without dependency");
	hStatus->Text = "download without dependency";
}


String^ WPFCppCliDemo::toSystemString(std::string& str)
{
  StringBuilder^ pStr = gcnew StringBuilder();
  for (size_t i = 0; i < str.size(); ++i)
    pStr->Append((Char)str[i]);
  return pStr->ToString();
}

void WPFCppCliDemo::addText(String^ msg)
{
  hTextBlock2->Text += msg + "\n\n";
  hTextBlock1->Text += msg + "\n\n";
}


void WPFCppCliDemo::getMessage()
{
  // recvThread runs this function

  while (true)
  {
    std::cout << "\n  receive thread calling getMessage()";
    std::string msg = pRecvr_->getMessage();
	msg = messageDispatch(msg);
    String^ sMsg = toSystemString(msg);
    array<String^>^ args = gcnew array<String^>(1);
    args[0] = sMsg;
    Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addText);
    Dispatcher->Invoke(act, args);  // must call addText on main UI thread
  }
}

std::string WPFCppCliDemo::messageDispatch(std::string msg) {
	return msg;
}
void WPFCppCliDemo::clear(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  cleared message text");
  hStatus->Text = "Cleared message";
  hTextBlock2->Text = "";
}


void WPFCppCliDemo::getItemsFromList(Object^ sender, RoutedEventArgs^ args)
{
	int index = 0;
	int count = hListBox->SelectedItems->Count;
	hStatus->Text = "Show Selected Items";
	//how you check out the files.
	array<System::String^>^ items = gcnew array<String^>(count);
	if (count > 0) {
		for each (String^ item in hListBox->SelectedItems)
		{
			items[index++] = item + "eeee";
			//In this place, we need to show the file item and its contents.
			hTextBlock2->Text += item + "\n";
		}
	}

	hListBox->Items->Clear();
	if (count > 0) {
		for each (String^ item in items)
		{
			hListBox->Items->Add(item);
		}
	}
}



void WPFCppCliDemo::setUpFileListView()
{
  Console::Write("\n  setting up FileList view");
  hFileListGrid->Margin = Thickness(20);
  hFileListTab->Content = hFileListGrid;
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hFileListGrid->RowDefinitions->Add(hRow1Def);
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hListBox->SelectionMode = SelectionMode::Multiple;
  hBorder1->Child = hListBox;
  hListBox->Width = 350;
  hTextBlock2->Padding = Thickness(15);
  hTextBlock2->Text = "";
  hTextBlock2->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
  hTextBlock2->FontWeight = FontWeights::Bold;
  hTextBlock2->FontSize = 16;
  hTextBlock2->Width = 350;
  Border^ text = gcnew Border();
  text->BorderThickness = Thickness(1);
  text->BorderBrush = Brushes::Black;
  text->Child = hTextBlock2;
  hStackPanel3->Orientation = Orientation::Horizontal;
  hStackPanel3->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  
  hStackPanel3->Children->Add(text);
  hStackPanel3->Children->Add(hBorder1);

  hFileListGrid->SetRow(hStackPanel3, 0);
  hFileListGrid->Children->Add(hStackPanel3);
  setUpFileListViewHelp();
}

void WPFCppCliDemo::setUpFileListViewHelp() {
  RowDefinition^ hRow2Def = gcnew RowDefinition();
  hRow2Def->Height = GridLength(75);
  RowDefinition^ hRow2Def2 = gcnew RowDefinition();
  hRow2Def2->Height = GridLength(75);
  RowDefinition^ hRow2Def3 = gcnew RowDefinition();
  hRow2Def->Height = GridLength(75);
  RowDefinition^ hRow2Def4 = gcnew RowDefinition();
  hRow2Def2->Height = GridLength(75);
  hFileListGrid->RowDefinitions->Add(hRow2Def);
  hFolderBrowseButton->Content = "Select Directory";
  hFolderBrowseButton->Height = 30;
  hFolderBrowseButton->Width = 120;
  hFolderBrowseButton->BorderThickness = Thickness(2);
  hFolderBrowseButton->BorderBrush = Brushes::Black;
  hShowItemsButton->Content = "Show Selected Items";
  hShowItemsButton->Height = 30;
  hShowItemsButton->Width = 120;
  hShowItemsButton->BorderThickness = Thickness(2);
  hShowItemsButton->BorderBrush = Brushes::Black;
  hSend->Content = "Send Message";
  hSend->Height = 30;
  hSend->Width = 120;
  hSend->BorderThickness = Thickness(2);
  hSend->BorderBrush = Brushes::Black;
  hClear->Content = "Clear";
  hClear->Height = 30;
  hClear->Width = 120;
  hClear->BorderThickness = Thickness(2);
  hClear->BorderBrush = Brushes::Black;
  hStackPanel2->Children->Add(hFolderBrowseButton);
  TextBlock^ hSpacer = gcnew TextBlock();
  hSpacer->Width = 10;
  hStackPanel2->Children->Add(hSpacer);
  hStackPanel2->Children->Add(hShowItemsButton);
  TextBlock^ hSpacer1 = gcnew TextBlock();
  hSpacer1->Width = 10;
  hStackPanel2->Children->Add(hSpacer1);
  hStackPanel2->Children->Add(hSend);
  TextBlock^ hSpacer2 = gcnew TextBlock();
  hSpacer2->Width = 10;
  hStackPanel2->Children->Add(hSpacer2);
  hStackPanel2->Children->Add(hClear);
  hStackPanel2->Orientation = Orientation::Horizontal;
  hStackPanel2->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  hFileListGrid->SetRow(hStackPanel2, 20);
  hFileListGrid->Children->Add(hStackPanel2);
  hFolderBrowserDialog->ShowNewFolderButton = false;
  hFolderBrowserDialog->SelectedPath = System::IO::Directory::GetCurrentDirectory();
}

void WPFCppCliDemo::browseForFolder(Object^ sender, RoutedEventArgs^ args)
{
  std::cout << "\n  Browsing for folder";
  hListBox->Items->Clear();
  System::Windows::Forms::DialogResult result;
  result = hFolderBrowserDialog->ShowDialog();
  if (result == System::Windows::Forms::DialogResult::OK)
  {
    String^ path = hFolderBrowserDialog->SelectedPath;
    std::cout << "\n  opening folder \"" << toStdString(path) << "\"";
    array<String^>^ files = System::IO::Directory::GetFiles(path, L"*.*");
	//At this place ,we can show the item we want to show.
    for (int i = 0; i < files->Length; ++i)
      hListBox->Items->Add(files[i]+"test");
    array<String^>^ dirs = System::IO::Directory::GetDirectories(path);
	for (int i = 0; i < dirs->Length; ++i)
	{
		hListBox->Items->Add(L"<> " + dirs[i]);
	}
  }
}
void WPFCppCliDemo::setUpConnectionView()
{
  Console::Write("\n  setting up Download view");
  hDownloadGrid->Margin = Thickness(20);
  hConnectTab ->Content = hDownloadGrid;
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hDownloadGrid->RowDefinitions->Add(hRow1Def);
  hDownloadGrid->SetRow(hDownloadBox, 0);
  hDownloadGrid->Children->Add(hDownloadBox);
  hDownloadBox->Width = 350;

  hGetFolder->Content = "Get Folder";
  hGetFolder->Height = 30;
  hGetFolder->Width = 120;
  hGetFolder->BorderThickness = Thickness(2);
  hGetFolder->BorderBrush = Brushes::Black;

  hDownloadWithD->Content = "Download With Dependency";
  hDownloadWithD->Height = 30;
  hDownloadWithD->Width = 120;
  hDownloadWithD->BorderThickness = Thickness(2);
  hDownloadWithD->BorderBrush = Brushes::Black;

  hDownloadWithoutD->Content = "Download Without Dependency";
  hDownloadWithoutD->Height = 30;
  hDownloadWithoutD->Width = 120;
  hDownloadWithoutD->BorderThickness = Thickness(2);
  hDownloadWithoutD->BorderBrush = Brushes::Black;

  hDownloadButton->Children->Add(hGetFolder);
  hDownloadButton->Children->Add(hDownloadWithD );
  hDownloadButton->Children->Add(hDownloadWithoutD);

  RowDefinition^ hRow1Def2 = gcnew RowDefinition();
  hDownloadGrid->RowDefinitions->Add(hRow1Def2);
  hDownloadGrid->SetRow(hDownloadButton, 1);
  hDownloadGrid->Children->Add(hDownloadButton);
}

void WPFCppCliDemo::OnLoaded(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  Window loaded");
}
void WPFCppCliDemo::Unloading(Object^ sender, System::ComponentModel::CancelEventArgs^ args)
{
  Console::Write("\n  Window closing");
}

[STAThread]
//int _stdcall WinMain()
int main(array<System::String^>^ args)
{
  Console::WriteLine(L"\n Starting WPFCppCliDemo");

  Application^ app = gcnew Application();
  app->Run(gcnew WPFCppCliDemo());
  Console::WriteLine(L"\n\n");
}
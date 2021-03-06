program BafangConfigTool;

uses
  Forms,
  Main in 'Main.pas' {MainForm},
  CommonFunctions in 'CommonFunctions.pas',
  About in 'About.pas' {AboutForm},
  Communication in 'Communication.pas',
  Warning in 'Warning.pas' {frmWarning},
  Help in 'Help.pas' {frmHelp};

{$R *.res}
{$R Win7GUI.RES}  // Windows 7 (Vista, 8, 10) GUI look

begin
  Application.Initialize;
  Application.Title := 'Bafang Configuration Tool';
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TfrmWarning, frmWarning);
  Application.CreateForm(TfrmHelp, frmHelp);
  //Application.CreateForm(TAboutForm, AboutForm);  // It is loaded dynamically so it is not called here
  Application.Run;
end.

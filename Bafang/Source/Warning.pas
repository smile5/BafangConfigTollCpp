unit Warning;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, IniFiles;

type
  TfrmWarning = class(TForm)
    btnCloseWarning: TButton;
    lblLine1: TLabel;
    lblRedParam: TLabel;
    lblPurplepParam: TLabel;
    lblBlackParam: TLabel;
    lblBlackParamText: TLabel;
    lblPurpleParamText: TLabel;
    lblRedParamText: TLabel;
    lblLanguage: TLabel;
    cbbLanguage: TComboBox;
    memWarning: TMemo;
    procedure btnCloseKeyPress(Sender: TObject; var Key: Char);
    procedure cbbLanguageChange(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure SetWarningLanguage;

  private
    { Déclarations privées }
  public
    { Déclarations publiques }
  end;

var
  frmWarning: TfrmWarning;
  procedure ShowWarning;
//  procedure SetWarningLanguage;


implementation

Uses Main, Help, CommonFunctions;

procedure ShowWarning; //procedure for creating and destroying the warning window

begin
  with TfrmWarning.Create(Application) do
    try
      cbbLanguage.Items:= MainForm.cbbLanguage.Items ;
      cbbLanguage.ItemIndex := MainForm.cbbLanguage.ItemIndex ;
      SetWarningLanguage;
      ShowModal;
    finally
      Free;
    end;
end;

{$R *.dfm}

procedure TfrmWarning.btnCloseKeyPress(Sender: TObject; var Key: Char);
begin
  if key=#27 then Close; // Close window if Esc is clicked
end;



procedure TfrmWarning.cbbLanguageChange(Sender: TObject);
begin
  SetDefaultLanguage(listLang.Strings[cbbLanguage.ItemIndex]);
  MainForm.cbbLanguage.ItemIndex := cbbLanguage.ItemIndex;
  MainForm.cbbLanguageChange(Sender);
  SetWarningLanguage;
end;

// Set    SetWarningLanguage
procedure TfrmWarning.SetWarningLanguage;
var
  langFile: TInifile;
  myFile: string;
begin
  myFile:=ExtractFilePath(Application.ExeName) + 'Languages\' + selectedLang + '\soft\warning.ini'  ;
  langFile:=TInifile.Create(myFile);
  Caption :=  langFile.ReadString('WARNING','title', 'Bafang Configuration Tool');
  lblLine1.Caption  :=  langFile.ReadString('WARNING','line1', 'Bafang Configuration Tool');
  memWarning.Clear ;
  memWarning.Lines.SetText (PAnsiChar(langFile.ReadString('WARNING','line2', 'Bafang Configuration Tool')));
  lblRedParam.Caption :=  langFile.ReadString('WARNING','redParam', 'Bafang Configuration Tool');
  lblRedParamText.Caption :=  langFile.ReadString('WARNING','redParamText', 'Bafang Configuration Tool');
  lblPurplepParam.Caption  :=  langFile.ReadString('WARNING','purpleParam', 'Bafang Configuration Tool');
  lblPurpleParamText.Caption   :=  langFile.ReadString('WARNING','purpleParamText', 'Bafang Configuration Tool');
  lblBlackParam.Caption   :=  langFile.ReadString('WARNING','blackParam', 'Bafang Configuration Tool');
  lblBlackParamText.Caption  :=  langFile.ReadString('WARNING','blackParamText', 'Bafang Configuration Tool');
  lblLanguage.Caption  :=  langFile.ReadString('WARNING','Language', 'Bafang Configuration Tool');
  btnCloseWarning.Caption   :=  langFile.ReadString('WARNING','btnClose', 'Bafang Configuration Tool');



end;

procedure TfrmWarning.FormCreate(Sender: TObject);
begin
   SetWarningLanguage;
end;

end.





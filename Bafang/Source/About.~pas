unit About;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ShellApi, ExtCtrls;

type
  TAboutForm = class(TForm)
    lbAppName: TLabel;
    lbOriginalAuthor: TLabel;
    lbAuthor: TLabel;
    lbOriginalInfo: TLabel;
    btnClose: TButton;
    lbPenoffLink: TLabel;
    lblVersion: TLabel;
    lblUpgradedBy: TLabel;
    Label2: TLabel;
    lblCyclurba: TLabel;
    lblHelpTips: TLabel;
    lblElecbike: TLabel;
    Label3: TLabel;
    Label1: TLabel;
    lblBafangLink: TLabel;
    Shape1: TShape;
    Shape2: TShape;
    procedure btnCloseKeyPress(Sender: TObject; var Key: Char);
    procedure lbPenoffLinkClick(Sender: TObject);
    procedure lblCyclurbaClick(Sender: TObject);
    procedure lblElecbikeClick(Sender: TObject);
    procedure lblBafangLinkClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  AboutForm: TAboutForm;

procedure ShowAbout;

implementation

Uses Main, IniFiles, CommonFunctions;

procedure ShowAbout; //procedure for creating and destroying the About window
var
  langFile: TInifile;
  myFile: string;
begin
   myFile:=ExtractFilePath(Application.ExeName) + 'Languages\' + selectedLang + '\soft\main.ini'  ;
  langFile:=TInifile.Create(myFile);


  with TAboutForm.Create(Application) do
    try
  Caption :=  langFile.ReadString('ABOUT','title', 'About');
  lblUpgradedBy.caption :=  langFile.ReadString('ABOUT','updagred', 'Upgraded by : ');
  lblHelpTips.caption :=  langFile.ReadString('ABOUT','helptips', 'Upgraded by : ');
  lbOriginalInfo.caption:=  langFile.ReadString('ABOUT','originalapp', 'Original application designed by ');
  btnClose.caption:=  langFile.ReadString('ABOUT','close', 'Close ');
     ShowModal;
    finally
      Free;
    end;
end;

{$R *.dfm}

procedure TAboutForm.btnCloseKeyPress(Sender: TObject; var Key: Char);
begin
  if key=#27 then Close; // Close window if Esc is clicked
end;

procedure TAboutForm.lbPenoffLinkClick(Sender: TObject);
begin
  ShellExecute(0, 'OPEN', PChar(lbPenoffLink.Caption), '', '', SW_SHOWNORMAL);  // Go to Penoff's website
end;

procedure TAboutForm.lblCyclurbaClick(Sender: TObject);
begin
  ShellExecute(0, 'OPEN', PChar(lblCyclurba.Caption), '', '', SW_SHOWNORMAL); 
end;

procedure TAboutForm.lblElecbikeClick(Sender: TObject);
begin
  ShellExecute(0, 'OPEN', 'https://electricbike-blog.com/2015/06/26/a-hackers-guide-to-programming-the-bbs02/', '', '', SW_SHOWNORMAL);
end;

procedure TAboutForm.lblBafangLinkClick(Sender: TObject);
begin
  ShellExecute(0, 'OPEN', PChar(lblBafangLink.Caption), '', '', SW_SHOWNORMAL);  // Go to Pafang website
end;

end.

unit Help;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmHelp = class(TForm)
    btnHelpClose: TButton;
    memHelp: TMemo;
  private
    { Déclarations privées }
  public
    { Déclarations publiques }
  end;

var
  frmHelp: TfrmHelp;

  procedure ShowHelp(section: String; item: String);
  procedure SetDefaultLanguage(language: string);

implementation

Uses Main, IniFiles, CommonFunctions ;
//, CommonFunctions, IniFiles  ;

procedure SetDefaultLanguage(language: string);
var
   langFile: TInifile;
begin
  selectedLang:=listLang.Strings[MainForm.cbbLanguage.ItemIndex];
  langFile:=TInifile.Create(ExtractFilePath(Application.ExeName) + 'Languages\languages.ini');
  langFile.WriteString('LANGUAGES', 'default',  selectedLang);
  langFile.Destroy ;
end;

procedure ShowHelp(section: String; item: String); //procedure for creating and destroying the help window
var
   myFile: string;
   helpFile: TInifile;
   titleHelp,textHelp,nameLine : string;
   nbLine, i: integer ;
   langFile: TInifile;
   myFile1: string;
begin
  myFile:=ExtractFilePath(Application.ExeName) + 'Languages\' + selectedLang + '\help\' + section + '.ini'  ;
  helpFile:=TInifile.Create(myFile);

  myFile1:=ExtractFilePath(Application.ExeName) + 'Languages\' + selectedLang + '\soft\main.ini'  ;
  langFile:=TInifile.Create(myFile1) ;

  titleHelp:=helpFile.ReadString (item,'title','Help');
  nbLine:=helpFile.ReadInteger(item,'nbLine',0);
  textHelp:='';
  for i := 1 to nbLine do
  begin
     nameLine:= 'line' + IntToStr(i);
     textHelp:=textHelp  + #13#10 + helpFile.ReadString (item,nameLine,'');
  end;

  with TfrmHelp.Create(Application) do
    try
     Caption:=titleHelp;
     btnHelpClose.Caption:= langFile.ReadString('HELP','close','Close');
     memHelp.Lines.Clear ;
     memHelp.Lines.Add(textHelp);
     //memHelp.Lines.Add(selectedLang);

     ShowModal;
    finally
      Free;
    end;
    helpFile.Destroy ;
    langFile.Destroy ;
end;

{$R *.dfm}

end.

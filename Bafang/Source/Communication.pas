unit Communication;

interface

uses
  Windows, SysUtils, Classes, Forms, Registry, Main, CommonFunctions;

  function  ListPorts: TStrings;                  // Lists all available COM ports
  procedure ConnectDev;                           // Connect to controller
  procedure RestartPort;                          // Restart COM port
  procedure ReadDev(block: byte);                 // Reads from controller
  procedure WriteDev(block: byte);                // Writes to controller
  procedure DecodeGEN(data_in: array of byte);    // Decode General data
  procedure DecodeBAS(data_in: array of byte);    // Decode Basic Settings data
  procedure DecodePAS(data_in: array of byte);    // Decode Pedal Assist Settings data
  procedure DecodeTHR(data_in: array of byte);    // Decode Throttle Handle Settings data
  procedure DecodeBASerr(data_in: array of byte); // Decode Basic error data
  procedure DecodePASerr(data_in: array of byte); // Decode Pedal Assist error data
  procedure DecodeTHRerr(data_in: array of byte); // Decode Throttle Handle error data

implementation

// Returns a list of all available COM ports
function ListPorts: TStrings;
var
  i:    integer;
  reg:  TRegistry;
  ts:   TStrings;
begin
  reg := TRegistry.Create(KEY_READ);  // Must use read only access in newer Windows versions or it won't work
  reg.RootKey := HKEY_LOCAL_MACHINE;
  reg.OpenKey('HARDWARE\DEVICEMAP\SERIALCOMM', false);
  ts := TStringList.Create;
  Result := TStringList.Create;
  reg.GetValueNames(ts);
  for i := 0 to ts.Count - 1 do // make the list
    begin
      Result.Add(reg.ReadString(ts.Strings[i]));
    end;
  ts.Free;
  reg.CloseKey;
  reg.free;
end;

// Connect to controller
procedure ConnectDev;
var
  byteSendData: array[0..12] of byte;
begin
  byteSendData[0] := $11; // Read command
  byteSendData[1] := $51; // Read General location
  byteSendData[2] := $04; // Data bytes count ???
  byteSendData[3] := $B0; // Unknown
  byteSendData[4] := (byteSendData[1] + byteSendData[2] + byteSendData[3]) Mod 256; // Data verification byte

  // Send command
  sleep(80);
  if not MainForm.CommPort.WriteCommData(@byteSendData,5) then
    Application.MessageBox(PAnsiChar(GetErrorText('err4')), PAnsiChar(GetErrorTitle('err4')), MB_ICONERROR + mb_OK)
  else // If not working try reconnecting to controller
    begin
      sleep(80);
      MainForm.CommPort.BaudRate  := Main.baudrate;
      MainForm.TimerConnect.Enabled := true;
    end;
end;

//Restart COM port
procedure RestartPort;
begin
  MainForm.CommPort.StopComm;
  MainForm.CommPort.StartComm;
end;

// Read location of memory from controller
procedure ReadDev(block: byte);
var
  byteSendData: array[0..255] of byte;
begin
  byteSendData[0] := $11;   // Read command
  byteSendData[1] := block; // Read location (0x5X)
  Sleep(50);                // Wait for controller to prepare for sending
  if not MainForm.CommPort.WriteCommData(@byteSendData, 2) then
    Application.MessageBox(PAnsiChar(GetErrorText('err5')), PAnsiChar(GetErrorTitle('err5')), MB_ICONERROR + mb_OK);
end;

// Writes to controller
procedure WriteDev(block: byte);
var
  byteSendData: array[0..255] of byte;
  i, st, size: integer ;
begin
  size := 0; // init var

  with MainForm do
  begin
    case block of
      BAS:begin // Basic settings
            byteSendData[0]  := $16; // Write command
            byteSendData[1]  := $52; // Write location - Basic block
            byteSendData[2]  := 24;  // 24 bytes of settings to be written to flash
            // Add Basic block settings
            byteSendData[3]  := StrToInt(cbbBasLowBat.text);
            byteSendData[4]  := speBasCurrLim.Value;
            byteSendData[5]  := speBasAssist0CurrLim.Value;
            byteSendData[6]  := speBasAssist1CurrLim.Value;
            byteSendData[7]  := speBasAssist2CurrLim.Value;
            byteSendData[8]  := speBasAssist3CurrLim.Value;
            byteSendData[9]  := speBasAssist4CurrLim.Value;
            byteSendData[10] := speBasAssist5CurrLim.Value;
            byteSendData[11] := speBasAssist6CurrLim.Value;
            byteSendData[12] := speBasAssist7CurrLim.Value;
            byteSendData[13] := speBasAssist8CurrLim.Value;
            byteSendData[14] := speBasAssist9CurrLim.Value;
            byteSendData[15] := speBasAssist0SpdLim.Value;
            byteSendData[16] := speBasAssist1SpdLim.Value;
            byteSendData[17] := speBasAssist2SpdLim.Value;
            byteSendData[18] := speBasAssist3SpdLim.Value;
            byteSendData[19] := speBasAssist4SpdLim.Value;
            byteSendData[20] := speBasAssist5SpdLim.Value;
            byteSendData[21] := speBasAssist6SpdLim.Value;
            byteSendData[22] := speBasAssist7SpdLim.Value;
            byteSendData[23] := speBasAssist8SpdLim.Value;
            byteSendData[24] := speBasAssist9SpdLim.Value;

            if cbbBasWheelDiam.ItemIndex = 12 then // if wheel size is 700C
              byteSendData[25] := 55
            else
              byteSendData[25] := StrToInt(cbbBasWheelDiam.text) * 2;

            if cbbBasSpdMtrType.ItemIndex = 2 then // if speed meter is by motor phase (for hub motors)
              byteSendData[26] := (3 * 64) + speBasSpdMtrSig.Value
            else
              byteSendData[26] := (cbbBasSpdMtrType.ItemIndex * 64) + speBasSpdMtrSig.Value;
            // End Add Basic block settings

            st := 0;
            for i := 0 to 25  do              // cycle from 0 to data lenght + 1 (24 + 1 = 25)
              st := st + byteSendData[i + 1]; // sum all data bytes including location and lenght bytes and excluding the command byte

            byteSendData[27] := st mod 256;   // Data verification byte - equals the remainder of sum of all data bytes divided by 256
            size := 28; // size to writre
          end;
      PAS:begin // Pedal Assist settings
            byteSendData[0] := $16; // Write command
            byteSendData[1] := $53; // Write location - PAS block
            byteSendData[2] := 11;  // 11 bytes of settings to be written to flash
            // Add PAS block settings
            byteSendData[3] := cbbPASPedalSensType.ItemIndex;

            if cbbPASDesigAssist.ItemIndex = 0 then // if PAS designated assist is set by LCD
              byteSendData[4] := 255
            else
              byteSendData[4] := cbbPASDesigAssist.ItemIndex - 1;

            if cbbPASSpdLim.ItemIndex = 0 then // if PAS speed limit is set by LCD
              byteSendData[5] := 255
            else
              byteSendData[5] := cbbPASSpdLim.ItemIndex + 14;

            byteSendData[6] := spePASStartCurr.Value;
            byteSendData[7] := cbbPASSlowStartMode.ItemIndex + 1;
            byteSendData[8] := spePASStartDeg.Value;

            if cbbPASWorkMode.ItemIndex = 0 then // if PAS work mode is undetermined
              byteSendData[9] := 255
            else
              byteSendData[9] := cbbPASWorkMode.ItemIndex + 9;

            byteSendData[10] := spePASStopDelay.Value;
            byteSendData[11] := spePASCurrDecay.Value;
            byteSendData[12] := spePASStopDecay.Value;
            byteSendData[13] := spePASKeepCurr.Value;
            // End Add PAS block settings

            st:=0;
            for i := 0 to 12  do              // cycle from 0 to data lenght + 1 (11 + 1 = 12)
              st := st + byteSendData[i + 1]; // sum all data bytes including location and lenght bytes and excluding the command byte

            byteSendData[14] := st mod 256;   // Data verification byte - equals the remainder of sum of all data bytes divided by 256
            size := 15; // size to writre
          end;
      THR:begin // Throttle Handle settings
            byteSendData[0] := $16; // Write command
            byteSendData[1] := $54; // Write location - Throttle block
            byteSendData[2] := 6;   // 6 bytes of settings to be written to flash
            // Add Throttle block settings
            byteSendData[3] := speThrStartVolt.Value;
            byteSendData[4] := speThrEndVolt.Value;
            byteSendData[5] := cbbThrMode.ItemIndex;

            if cbbThrAssistLvl.ItemIndex = 0 then // if Throttle assist level is set by LCD
              byteSendData[6] := 255
            else
              byteSendData[6] := cbbThrAssistLvl.ItemIndex - 1;

            if edThrSpeedLim.ItemIndex = 0 then // if Throttle speed limit is set by LCD
              byteSendData[7] := 255
            else
              byteSendData[7] :=edThrSpeedLim.ItemIndex + 14;

            byteSendData[8] := speThrStartCurr.Value;
            // End Add Throttle block settings

            st:=0;
            for i := 0 to 7  do               // cycle from 0 to data lenght + 1 (6 + 1 = 7)
              st := st + byteSendData[i + 1]; // sum all data bytes including location and lenght bytes and excluding the command byte

            byteSendData[9] := st  Mod 256;   // Data verification byte - equals the remainder of sum of all data bytes divided by 256
            size := 10; // size to writre
          end; // End throttle
    end; // end case

    Sleep(50);
    if not CommPort.WriteCommData(@byteSendData, size) then
      Application.MessageBox(PAnsiChar(GetErrorText('err6')), PAnsiChar(GetErrorTitle('err6')), MB_ICONERROR + mb_OK);
  end;
end;

// Decode General data
procedure DecodeGEN(data_in: array of byte);
var
  i: integer; // counter
begin
  with MainForm do // link to MainForm
  begin
    TimerConnect.Enabled := false;

    lbManuf.Caption := ByteArrToStr(data_in, 2, 4); // Manufacturer
    lbModel.Caption := ByteArrToStr(data_in, 6, 4); // Model
    lbHWVer.Caption := 'V' + Chr(data_in[10]) + '.' + Chr(data_in[11]); // Hardware verion
    lbFWVer.Caption := 'V' + Chr(data_in[12]) + '.' + Chr(data_in[13]) + '.' + Chr(data_in[14]) + '.' + Chr(data_in[15]); // Firmware version

    cbbBasLowBat.Items.Clear;
    case data_in[16] of // Nominal voltage
      0:begin
          lbNomVolt.Caption := '24V';
          for i := 18 to 22 do
            cbbBasLowBat.Items.Add(IntToStr(i));
        end;
      1:begin
          lbNomVolt.Caption := '36V';
          for i := 28 to 32 do
            cbbBasLowBat.Items.Add(IntToStr(i));
        end;
      2:begin
          lbNomVolt.Caption := '48V';
          for i := 38 to 43 do
            cbbBasLowBat.Items.Add(IntToStr(i));
        end;
      3:begin
          lbNomVolt.Caption := '60V';
          for i := 48 to 55 do
            cbbBasLowBat.Items.Add(IntToStr(i));
        end;
      4:begin
          lbNomVolt.Caption := '24-48V';
          for i := 18 to 43 do
            cbbBasLowBat.Items.Add(IntToStr(i));
        end;
      else
      begin
        lbNomVolt.Caption := '24-60V';
        for i := 18 to 55 do
          cbbBasLowBat.Items.Add(IntToStr(i));
      end;
    end;  // Nominal Voltage case end

    lbMaxCurr.Caption := IntToStr(data_in[17]) + 'A';

    speBasCurrLim.MaxValue := data_in[17]; // limit control

    // Enable read and write buttons and disconnect
    btnPortDisconnect.Enabled := true;
    btnThrWrite.Enabled       := true;
    btnThrRead.Enabled        := true;
    btnPASWrite.Enabled       := true;
    btnPASRead.Enabled        := true;
    btnBasWrite.Enabled       := true;
    btnBasRead.Enabled        := true;
    btnWriteFlash.Enabled     := true;
    btnReadFlash.Enabled      := true;
    next_op                   := rdIgnore; // ignore further data
  end;

  RestartPort;
end;

// Decode Basic Settings data
procedure DecodeBAS(data_in: array of byte);
var
  i, t, m: integer;
begin
  with MainForm do // link to MainForm
  begin
    // Low Battery list changes according to controller. If different file loaded (max 22V instead of 43V for example) then regenerate list
    cbbBasLowBat.Items.Clear;
    if lbNomVolt.Caption = '24V' then    for i := 18 to 22 do cbbBasLowBat.Items.Add(IntToStr(i));
    if lbNomVolt.Caption = '36V' then    for i := 28 to 32 do cbbBasLowBat.Items.Add(IntToStr(i));
    if lbNomVolt.Caption = '48V' then    for i := 38 to 43 do cbbBasLowBat.Items.Add(IntToStr(i));
    if lbNomVolt.Caption = '60V' then    for i := 48 to 55 do cbbBasLowBat.Items.Add(IntToStr(i));
    if lbNomVolt.Caption = '24-48V' then for i := 18 to 43 do cbbBasLowBat.Items.Add(IntToStr(i));
    if lbNomVolt.Caption = '24-60V' then for i := 18 to 55 do cbbBasLowBat.Items.Add(IntToStr(i));

    cbbBasLowBat.ItemIndex        := cbbBasLowBat.Items.IndexOf(IntToStr(data_in[2]));
    speBasCurrLim.Value           := data_in[3];
    speBasAssist0CurrLim.Value    := data_in[4];
    speBasAssist1CurrLim.Value    := data_in[5];
    speBasAssist2CurrLim.Value    := data_in[6];
    speBasAssist3CurrLim.Value    := data_in[7];
    speBasAssist4CurrLim.Value    := data_in[8];
    speBasAssist5CurrLim.Value    := data_in[9];
    speBasAssist6CurrLim.Value    := data_in[10];
    speBasAssist7CurrLim.Value    := data_in[11];
    speBasAssist8CurrLim.Value    := data_in[12];
    speBasAssist9CurrLim.Value    := data_in[13];
    speBasAssist0SpdLim.Value     := data_in[14];
    speBasAssist1SpdLim.Value     := data_in[15];
    speBasAssist2SpdLim.Value     := data_in[16];
    speBasAssist3SpdLim.Value     := data_in[17];
    speBasAssist4SpdLim.Value     := data_in[18];
    speBasAssist5SpdLim.Value     := data_in[19];
    speBasAssist6SpdLim.Value     := data_in[20];
    speBasAssist7SpdLim.Value     := data_in[21];
    speBasAssist8SpdLim.Value     := data_in[22];
    speBasAssist9SpdLim.Value     := data_in[23];

    // Convert wheel size
    t := data_in[24] mod 2;
    m := (data_in[24] mod 2) + (data_in[24] div 2);
    if (m > 27) then
      begin
        if (m = 28) and (t = 1) then
          cbbBasWheelDiam.ItemIndex := 12
        else
          cbbBasWheelDiam.ItemIndex := m - 15;
      end
    else
      cbbBasWheelDiam.ItemIndex := m - 16;

    // Convert speed meter type and signals
    m := data_in[25] div 64;
    if m = 3 then
      cbbBasSpdMtrType.ItemIndex := 2
    else
      cbbBasSpdMtrType.ItemIndex := m;
    speBasSpdMtrSig.Value := data_in[25] mod 64;

    RestartPort;

    if (next_op = rdSingle) then
      begin
        next_op := rdIgnore; // ignore further data
        Application.MessageBox(PAnsiChar(GetErrorText('err7')), PAnsiChar(GetErrorTitle('err7')), mb_ICONINFORMATION+mb_Ok);
      end
    else
      begin
        next_op := rdAll;
        ReadDev(PAS);
      end;
  end;
end;

// Decode Basic error data
procedure DecodeBASerr(data_in: array of byte);
begin
  with MainForm do // link to MainForm
  begin
    case data_in[1] of
      0:begin //Low Battery Protect
          Application.MessageBox(PAnsiChar(GetErrorText('err8')), PAnsiChar(GetErrorTitle('err8')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          cbbBasLowBat.SetFocus;
        end;
      1:begin //Current Limit
          Application.MessageBox(PAnsiChar(GetErrorText('err9')), PAnsiChar(GetErrorTitle('err9')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasCurrLim.SetFocus;
        end;
      2:begin //Current Limit 0
          Application.MessageBox(PAnsiChar(GetErrorText('err10')), PAnsiChar(GetErrorTitle('err10')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex :=0 ;
          speBasAssist0CurrLim.SetFocus;
        end;
      4:begin //Current Limit 1
          Application.MessageBox(PAnsiChar(GetErrorText('err11')), PAnsiChar(GetErrorTitle('err11')) ,mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist1CurrLim.SetFocus;
        end;
      6:begin //Current Limit 2
          Application.MessageBox(PAnsiChar(GetErrorText('err12')), PAnsiChar(GetErrorTitle('err12')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist2CurrLim.SetFocus;
        end;
      8:begin //Current Limit 3
          Application.MessageBox(PAnsiChar(GetErrorText('err13')), PAnsiChar(GetErrorTitle('err13')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist3CurrLim.SetFocus;
        end;
      10:begin //Current Limit 4
          Application.MessageBox(PAnsiChar(GetErrorText('err14')), PAnsiChar(GetErrorTitle('err14')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist4CurrLim.SetFocus;
        end;
      12:begin //Current Limit 5
           Application.MessageBox(PAnsiChar(GetErrorText('err15')), PAnsiChar(GetErrorTitle('err15')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist5CurrLim.SetFocus;
         end;
      14:begin //Current Limit 6
           Application.MessageBox(PAnsiChar(GetErrorText('err16')), PAnsiChar(GetErrorTitle('err16')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist6CurrLim.SetFocus;
         end;
      16:begin //Current Limit 7
           Application.MessageBox(PAnsiChar(GetErrorText('err17')), PAnsiChar(GetErrorTitle('err17')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist7CurrLim.SetFocus;
         end;
      18:begin //Current Limit 8
           Application.MessageBox(PAnsiChar(GetErrorText('err18')), PAnsiChar(GetErrorTitle('err18')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist8CurrLim.SetFocus;
         end;
      20:begin //Current Limit 9
           Application.MessageBox(PAnsiChar(GetErrorText('err19')), PAnsiChar(GetErrorTitle('err19')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist9CurrLim.SetFocus;
         end;
      3:begin //Limit SPD0
          Application.MessageBox(PAnsiChar(GetErrorText('err20')), PAnsiChar(GetErrorTitle('err20')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist0SpdLim.SetFocus;
        end;
      5:begin //Limit SPD1
          Application.MessageBox(PAnsiChar(GetErrorText('err21')), PAnsiChar(GetErrorTitle('err21')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist1SpdLim.SetFocus;
        end;
      7:begin //Limit SPD2
          Application.MessageBox(PAnsiChar(GetErrorText('err22')), PAnsiChar(GetErrorTitle('err22')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist2SpdLim.SetFocus;
        end;
      9:begin //Limit SPD3
          Application.MessageBox(PAnsiChar(GetErrorText('err23')), PAnsiChar(GetErrorTitle('err23')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 0;
          speBasAssist3SpdLim.SetFocus;
        end;
      11:begin //Limit SPD4
           Application.MessageBox(PAnsiChar(GetErrorText('err24')), PAnsiChar(GetErrorTitle('err24')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist4SpdLim.SetFocus;
         end;
      13:begin //Limit SPD5
           Application.MessageBox(PAnsiChar(GetErrorText('err25')), PAnsiChar(GetErrorTitle('err25')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist5SpdLim.SetFocus;
         end;
      15:begin //Limit SPD6
           Application.MessageBox(PAnsiChar(GetErrorText('err26')), PAnsiChar(GetErrorTitle('err26')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist6SpdLim.SetFocus;
         end;
      17:begin //Limit SPD7
           Application.MessageBox(PAnsiChar(GetErrorText('err27')), PAnsiChar(GetErrorTitle('err27')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist7SpdLim.SetFocus;
         end;
      19:begin //Limit SPD8
           Application.MessageBox(PAnsiChar(GetErrorText('err28')), PAnsiChar(GetErrorTitle('err28')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist8SpdLim.SetFocus;
         end;
      21:begin //Limit SPD9
           Application.MessageBox(PAnsiChar(GetErrorText('err29')), PAnsiChar(GetErrorTitle('err29')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasAssist9SpdLim.SetFocus;
         end;
      22:begin //Wheel Diameter
           Application.MessageBox(PAnsiChar(GetErrorText('err30')), PAnsiChar(GetErrorTitle('err30')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           cbbBasWheelDiam.SetFocus;
         end;
      23:begin //SpdMeter Signal
           Application.MessageBox(PAnsiChar(GetErrorText('err31')), PAnsiChar(GetErrorTitle('err31')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 0;
           speBasSpdMtrSig.SetFocus;
         end;
      24:begin
           if (next_op = wrAll) then
             begin   // Flash write error
               RestartPort;
               WriteDev(PAS);
             end
             else
               Application.MessageBox(PAnsiChar(GetErrorText('err32')), PAnsiChar(GetErrorTitle('err32')), mb_ICONINFORMATION+mb_Ok);
         end;
    end; // case
    if (next_op = wrSingle) then
    begin   // Block write error
      next_op := rdIgnore; // ignore further data
      RestartPort;
    end;
  end;
end;

// Decode Pedal Assist Settings data
procedure DecodePAS(data_in: array of byte);
begin
  with MainForm do // link to MainForm
  begin
    cbbPASPedalSensType.ItemIndex := data_in[2];
    if data_in[3] = 255 then
       cbbPASDesigAssist.ItemIndex := 0
    else
       cbbPASDesigAssist.ItemIndex := data_in[3] + 1;

    if data_in[4] = 255 then
       cbbPASSpdLim.ItemIndex := 0
    else
       cbbPASSpdLim.ItemIndex := data_in[4] - 14;

    spePASStartCurr.Value := data_in[5];
    cbbPASSlowStartMode.ItemIndex := data_in[6] - 1;
    spePASStartDeg.Value := data_in[7];
    if data_in[8] = 255 then
       cbbPASWorkMode.ItemIndex := 0
    else
      cbbPASWorkMode.ItemIndex := data_in[8] - 9;

    spePASStopDelay.Value := data_in[9];
    spePASCurrDecay.Value := data_in[10];
    spePASStopDecay.Value := data_in[11];
    spePASKeepCurr.Value  := data_in[12];

    RestartPort;

    if (next_op = rdSingle) then
      begin
        next_op := rdIgnore; // ignore further data
        Application.MessageBox(PAnsiChar(GetErrorText('err33')), PAnsiChar(GetErrorTitle('err33')), mb_ICONINFORMATION+mb_Ok);
      end
    else
      begin
        next_op := rdAll;
        ReadDev(THR);
      end;
  end;
end;

// Decode Pedal Assist error data
procedure DecodePASerr(data_in: array of byte);
begin
  with MainForm do // link to MainForm
  begin
    case data_in[1] of
      0:begin //Pedal Sensor Type
          Application.MessageBox(PAnsiChar(GetErrorText('err34')), PAnsiChar(GetErrorTitle('err34')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          cbbPASPedalSensType.SetFocus;
        end;
      1:begin //Designated Assist Level
          Application.MessageBox(PAnsiChar(GetErrorText('err35')), PAnsiChar(GetErrorTitle('err35')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          cbbPASDesigAssist.SetFocus;
        end;
      2:begin //Speed Limit
          Application.MessageBox(PAnsiChar(GetErrorText('err36')), PAnsiChar(GetErrorTitle('err36')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          cbbPASSpdLim.SetFocus;
        end;
      3:begin //Start Current
          Application.MessageBox(PAnsiChar(GetErrorText('err37')), PAnsiChar(GetErrorTitle('err37')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          spePASStartCurr.SetFocus;
        end;
      4:begin //Slow-start Mode
          Application.MessageBox(PAnsiChar(GetErrorText('err38')), PAnsiChar(GetErrorTitle('err38')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          cbbPASSlowStartMode.SetFocus;
        end;
      5:begin //Start Degree
          Application.MessageBox(PAnsiChar(GetErrorText('err39')), PAnsiChar(GetErrorTitle('err39')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          spePASStartDeg.SetFocus;
        end;
      6:begin //Work Mode
          Application.MessageBox(PAnsiChar(GetErrorText('err40')), PAnsiChar(GetErrorTitle('err40')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          cbbPASWorkMode.SetFocus;
        end;
      7:begin //Stop Delay
          Application.MessageBox(PAnsiChar(GetErrorText('err41')), PAnsiChar(GetErrorTitle('err41')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          spePASStopDelay.SetFocus;
        end;
      8:begin //Current Decay
          Application.MessageBox(PAnsiChar(GetErrorText('err42')), PAnsiChar(GetErrorTitle('err42')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          spePASCurrDecay.SetFocus;
        end;
      9:begin //Stop Decay
          Application.MessageBox(PAnsiChar(GetErrorText('err43')), PAnsiChar(GetErrorTitle('err43')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 1;
          spePASStopDecay.SetFocus;
        end;
      10:begin //Keep Current
           Application.MessageBox(PAnsiChar(GetErrorText('err44')), PAnsiChar(GetErrorTitle('err44')), mb_ICONWARNING+mb_Ok);
           pctrlProfile.ActivePageIndex := 1;
           spePASKeepCurr.SetFocus;
         end;
      11:begin
           if (next_op = wrAll) then
           begin   // Flash write error
             RestartPort;
             WriteDev(THR);
           end;
         end;
    end; // case
    if (next_op = wrSingle) then
    begin
      next_op := rdIgnore; // ignore further data
      RestartPort;
      Application.MessageBox(PAnsiChar(GetErrorText('err45')), PAnsiChar(GetErrorTitle('err45')), mb_ICONINFORMATION+mb_Ok);
    end;
  end;
end;

// Decode Throttle Handle Settings data
procedure DecodeTHR(data_in: array of byte);
begin
  with MainForm do // link to MainForm
  begin
    speThrStartVolt.Value := data_in[2];
    speThrEndVolt.Value   := data_in[3];
    cbbThrMode.ItemIndex  := data_in[4];

    if data_in[5] = 255 then
      cbbThrAssistLvl.ItemIndex := 0
    else
      cbbThrAssistLvl.ItemIndex := data_in[5] + 1;

    if data_in[6] = 255 then
       edThrSpeedLim.ItemIndex := 0
    else
      edThrSpeedLim.ItemIndex := data_in[6] - 14;

    speThrStartCurr.Value := data_in[7];

    RestartPort;

    if (next_op = rdSingle) then
      Application.MessageBox(PAnsiChar(GetErrorText('err46')), PAnsiChar(GetErrorTitle('err46')), mb_ICONINFORMATION+mb_Ok)
    else
      Application.MessageBox(PAnsiChar(GetErrorText('err47')), PAnsiChar(GetErrorTitle('err47')), mb_ICONINFORMATION+mb_Ok);
      next_op := rdIgnore; // ignore further data
  end;
end;

// Decode Throttle Handle error data
procedure DecodeTHRerr(data_in: array of byte);
begin
  with MainForm do // link to MainForm
  begin
    case data_in[1] of
      0:begin //Start Voltage
          Application.MessageBox(PAnsiChar(GetErrorText('err48')), PAnsiChar(GetErrorTitle('err48')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 2;
          speThrStartVolt.SetFocus;
        end;
      1:begin //End Voltage
          Application.MessageBox(PAnsiChar(GetErrorText('err49')), PAnsiChar(GetErrorTitle('err49')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 2;
          speThrEndVolt.SetFocus;
        end;
      2:begin //Mode
          Application.MessageBox(PAnsiChar(GetErrorText('err50')), PAnsiChar(GetErrorTitle('err50')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 2;
          cbbPort.SetFocus;
        end;
      3:begin //Designated Assist
          Application.MessageBox(PAnsiChar(GetErrorText('err51')), PAnsiChar(GetErrorTitle('err51')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 2;
          cbbPort.SetFocus;
        end;
      4:begin //Speed Limit
          Application.MessageBox(PAnsiChar(GetErrorText('err52')), PAnsiChar(GetErrorTitle('err52')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 2;
          cbbPort.SetFocus;
        end;
      5:begin //Start Current
          Application.MessageBox(PAnsiChar(GetErrorText('err53')), PAnsiChar(GetErrorTitle('err53')), mb_ICONWARNING+mb_Ok);
          pctrlProfile.ActivePageIndex := 2;
          speThrStartCurr.SetFocus;
        end;
      6:begin
          RestartPort;

          if (next_op = wrSingle) then
            Application.MessageBox(PAnsiChar(GetErrorText('err54')), PAnsiChar(GetErrorTitle('err54')), mb_ICONINFORMATION+mb_Ok)
          else
            Application.MessageBox(PAnsiChar(GetErrorText('err55')), PAnsiChar(GetErrorTitle('err55')), mb_ICONINFORMATION+mb_Ok);
            next_op := rdIgnore; // ignore further data
        end;
    end; //case
  end;
end;


end.

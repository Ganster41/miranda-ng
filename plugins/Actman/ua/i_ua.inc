{}
function AddUAction(idx:integer; ptr:pChain):integer;
var
  buf:array [0..127] of AnsiChar;
begin
  if idx<0 then idx:=Length(UActionList);
  if idx=Length(UActionList) then
    SetLength(UActionList,Length(UActionList)+1);

  FillChar(UActionList[idx],SizeOf(tMyActionItem),0);

  with UActionList[idx] do
  begin
    // get Action settings
    dwActID:=ptr^.id;
    if (ptr^.flags and ACCF_DISABLED)<>0 then
      flags:=UAF_DISABLED;
    StrDupW(szActDescr,ptr^.descr);
    wSortIndex:=idx;
    
    // prepare for work
    IntToStr(StrCopyE(buf,'Actions/Action_'),ptr^.id);
    StrDup(szNameID,@buf);
    AddIcolibIcon (UActionList[idx]);
  end;

  SetLength(arMenuRec,Length(UActionList)+1);
  FillChar (arMenuRec[HIGH(arMenuRec)],SizeOf(tuaMenuRecA),0);
  result:=idx;
end;

function CreateUActionList:integer;
var
  ptr,ptr1:pChain;
  i:integer;
begin
  result:=CallService(MS_ACT_GETLIST,0,LPARAM(@ptr));
  SetLength(UActionList,result);

  SetLength(arMenuRec, result+1);
  FillChar (arMenuRec[0],(result+1)*SizeOf(tuaMenuRecA),0);

  if result>0 then
  begin
    ptr1:=ptr;
    inc(pbyte(ptr),4);
    for i:=0 to result-1 do
    begin
      AddUAction(i,ptr);
      LoadUA(i); // just here coz at list changes for new we don't have settings
      if (UActionList[i].flags and UAF_2STATE)<>0 then
        AddIcolibIconP(UActionList[i]);
      SetAllActionUsers(UActionList[i],true);
      inc(ptr);
    end;
    CallService(MS_ACT_FREELIST,0,LPARAM(ptr1));
  end;
end;

function ActListChange(wParam:WPARAM;lParam:LPARAM):integer; cdecl;
var
  ptr,ptr1:pChain;
  idx,i,j,count:integer;
  bFound:boolean;
begin
  result:=0;

  count:=CallService(MS_ACT_GETLIST,0,TLPARAM(@ptr));

  if count>0 then
  begin
    ptr1:=ptr;
    inc(pbyte(ptr),4);
    // maybe add ACTM_RELOAD (as NEW and DELETE) here too?
    if (wParam and (ACTM_NEW or ACTM_RENAME or ACTM_SORT or ACTM_DELETE))<>0 then
      for i:=0 to count-1 do
      begin
        // search corresponding element
        idx:=-1;
        for j:=0 to HIGH(UActionList) do
        begin
          if UActionList[j].dwActID=ptr^.id then
          begin
            idx:=j;
            break;
          end;
        end;
        // if we have no item in list for this action - then add new one
        if idx<0 then
          AddUAction(-1,ptr)
        else
        begin
          if (wParam and ACTM_RENAME)<>0 then
          begin
            // check for time economy - no need to change ALL items
            if StrCmpW(UActionList[idx].szActDescr,ptr^.descr)<>0 then
            begin
              mFreeMem(UActionList[idx].szActDescr);
              StrDupW (UActionList[idx].szActDescr,ptr^.descr);
            end;
          end;

          if (wParam and (ACTM_SORT or ACTM_DELETE or ACTM_NEW))<>0 then
            UActionList[idx].wSortIndex:=i;
        end;
        inc(ptr);
      end;
  end
  else
    ptr1:=nil;

  // now search deleted items
  if (wParam and ACTM_DELETE)<>0 then
  begin
    for j:=HIGH(UActionList) downto 0 do
    begin
      bFound:=false;
      if count>0 then
      begin
        ptr:=ptr1;
        inc(pbyte(ptr),4);
        for i:=0 to count-1 do
        begin
          if UActionList[j].dwActID=ptr^.id then
          begin
            bFound:=true;
            break;
          end;
          inc(ptr);
        end;
      end;
      if not bFound then
        DeleteUAction(j);
    end;
  end;

  if count>0 then
    CallService(MS_ACT_FREELIST,0,TLPARAM(ptr1));

  if settings<>0 then
  begin
    FillActionList(settings);
    ShowAction(settings,-1);
    SendMessage(GetParent(settings),PSM_CHANGED,0,0);
  end
  else
  begin
    SaveUAs;
    FillChar(arMenuRec[0],Length(arMenuRec)*SizeOf(tuaMenuRecA),0);
    for i:=0 to HIGH(UActionList) do
    begin
      SetAllActionUsers(UActionList[i],false);
    end;
  end;
end;

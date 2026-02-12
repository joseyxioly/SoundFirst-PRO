-- @description Dump Focused FX Parameters to Clipboard
-- @version 1.0
-- @author SoundFirst
-- @about
--   Dumps the parameters of the currently focused (or first) FX on the selected track
--   to the clipboard. Paste this into the SoundFirst PRO Mapper.

function Msg(str)
  reaper.ShowConsoleMsg(tostring(str) .. "\n")
end

function GetFocusedFX()
  -- 1. Try to find actual focused FX
  local retval, trackidx, itemidx, fxidx = reaper.GetFocusedFX()
  
  -- retval: 0=no focus, 1=track fx, 2=item fx
  if retval == 1 then
    local track
    if trackidx == 0 then
      track = reaper.GetMasterTrack(0)
    else
      track = reaper.GetTrack(0, trackidx - 1)
    end
    return track, fxidx
  end
  
  -- 2. Fallback: Selected Track, First FX
  local track = reaper.GetSelectedTrack(0, 0)
  if track then
    return track, 0
  end
      
  return nil, -1
end

function Main()
  reaper.ClearConsole()
  
  local track, fx_idx = GetFocusedFX()
  
  if not track then
    reaper.ShowMessageBox("No Track Selected or FX Focused.", "Dump Error", 0)
    return
  end
  
  -- Get FX Name
  local retval, fx_name = reaper.TrackFX_GetFXName(track, fx_idx, "")
  if not retval then
    reaper.ShowMessageBox("Could not get FX Name.", "Dump Error", 0)
    return
  end
  
  -- Count Params
  local num_params = reaper.TrackFX_GetNumParams(track, fx_idx)
  
  -- Prepare Output
  local output = ""
  output = output .. "[" .. fx_name .. "]\n"
  
  for i = 0, num_params - 1 do
    local retval, p_name = reaper.TrackFX_GetParamName(track, fx_idx, i, "")
    output = output .. "; " .. tostring(i) .. " = " .. p_name .. "\n"
  end
  
  -- Copy to Clipboard
  -- CF_SetClipboard is from SWS, but reaper.CF_SetClipboard might not exist if SWS is missing.
  -- Better to use a hidden text window trick or simple console output if SWS is missing?
  -- Wait, standard Reaper doesn't have SetClipboard? 
  -- Actually, implementing a simple hack: Show in Console and Select All? 
  -- Or just use a hidden temp file passed to 'clip'?
  -- Lua has os.execute.
  
  -- Trying os.execute clip method for Windows
  local file = io.open(os.getenv("TEMP") .. "\\reaper_clipboard_temp.txt", "w")
  file:write(output)
  file:close()
  os.execute("type \"" .. os.getenv("TEMP") .. "\\reaper_clipboard_temp.txt\" | clip")
  
  -- Also show in console just in case
  Msg("--- Parameters Copied to Clipboard ---")
  Msg(output)
  Msg("--------------------------------------")
  
  reaper.ShowMessageBox("Parameters for '" .. fx_name .. "' copied to Clipboard!", "Dump Success", 0)
end

Main()

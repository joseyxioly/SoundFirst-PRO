import wx
import wx.dataview
import configparser
import os
import re

# --- DATA STRUCTURES ---

MAPPABLE_BUTTONS = [
    "LOOP", "METRO", "TEMPO", # Transport/Global
    "IDEAS", "QUANTIZE", "AUTO", # Global/Edit
    "MUTE", "SOLO" # New
    # PLAY, STOP, REC are RESERVED
]

SPECIAL_ACTIONS = {
    "@REPORT_PEAK": "Read Track Peak",
    "@REPORT_GR": "Read Gain Reduction (GR)"
}

class Parameter:
    def __init__(self, pid, name):
        self.pid = pid
        self.name = name
    
    def __repr__(self):
        return f"{self.pid}: {self.name}"

class KnobMapping:
    def __init__(self):
        self.param_id = -1
        self.shift_param_id = -1
        self.touch_param_id = -1

class Page:
    def __init__(self, name="Page 1"):
        self.name = name
        self.knobs = [KnobMapping() for _ in range(8)]

class FXMapping:
    def __init__(self):
        self.plugin_name = "Unknown Plugin"
        self.pages = [Page()]
        self.buttons = {k: -1 for k in MAPPABLE_BUTTONS}
        self.params = {} # pid -> name

# --- WIZARD PANELS ---

class StartPanel(wx.Panel):
    def __init__(self, parent, on_next):
        super().__init__(parent)
        self.on_next = on_next
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        
        sizer.AddStretchSpacer()
        
        title = wx.StaticText(self, label="KompleteReaper FX Mapper")
        font = title.GetFont()
        font.SetPointSize(16)
        font.MakeBold()
        title.SetFont(font)
        sizer.Add(title, 0, wx.ALIGN_CENTER | wx.BOTTOM, 20)
        
        btn_new = wx.Button(self, label="Create New Mapping")
        btn_new.Bind(wx.EVT_BUTTON, lambda e: self.on_next("new"))
        sizer.Add(btn_new, 0, wx.ALIGN_CENTER | wx.ALL, 10)
        
        btn_edit = wx.Button(self, label="Edit Existing Mapping (.ini)")
        btn_edit.Bind(wx.EVT_BUTTON, lambda e: self.on_next("edit"))
        sizer.Add(btn_edit, 0, wx.ALIGN_CENTER | wx.ALL, 10)
        
        sizer.AddStretchSpacer()
        self.SetSizer(sizer)
        
        # Accessibility: Focus first button
        wx.CallAfter(btn_new.SetFocus)

class DumpPanel(wx.Panel):
    def __init__(self, parent, on_next, on_back):
        super().__init__(parent)
        self.on_next = on_next
        self.on_back = on_back
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        
        lbl = wx.StaticText(self, label="Step 1: Get Plugin Data\n\n1. Open REAPER.\n2. Select Track & Focus Plugin.\n3. Run action '@DUMP_FX'.\n4. Click 'Paste Dump' below.")
        sizer.Add(lbl, 0, wx.ALL, 10)
        
        self.txt_dump = wx.TextCtrl(self, style=wx.TE_MULTILINE)
        sizer.Add(self.txt_dump, 1, wx.EXPAND | wx.ALL, 10)
        
        btn_paste = wx.Button(self, label="Paste Dump from Clipboard")
        btn_paste.Bind(wx.EVT_BUTTON, self.on_paste)
        sizer.Add(btn_paste, 0, wx.ALIGN_CENTER | wx.ALL, 5)
        
        nav_sizer = wx.BoxSizer(wx.HORIZONTAL)
        btn_back = wx.Button(self, label="< Back")
        btn_back.Bind(wx.EVT_BUTTON, lambda e: self.on_back())
        btn_next = wx.Button(self, label="Next >")
        btn_next.Bind(wx.EVT_BUTTON, self.on_next_click)
        
        nav_sizer.Add(btn_back, 0, wx.ALL, 5)
        nav_sizer.AddStretchSpacer()
        nav_sizer.Add(btn_next, 0, wx.ALL, 5)
        
        sizer.Add(nav_sizer, 0, wx.EXPAND | wx.ALL, 5)
        self.SetSizer(sizer)

        # Accessibility: Focus text area for pasting
        wx.CallAfter(self.txt_dump.SetFocus)

    def on_paste(self, evt):
        if wx.TheClipboard.Open():
            do = wx.TextDataObject()
            if wx.TheClipboard.GetData(do):
                self.txt_dump.SetValue(do.GetText())
            wx.TheClipboard.Close()

    def on_next_click(self, evt):
        text = self.txt_dump.GetValue().strip()
        if not text:
            wx.MessageBox("Please paste the dump first.", "Error")
            return
        self.on_next(text)

class EditorPanel(wx.Panel):
    def __init__(self, parent, mapping, on_back, on_save):
        super().__init__(parent)
        self.mapping = mapping
        self.on_back = on_back
        self.on_save = on_save
        
        # --- UI LAYOUT (3 Columns) ---
        main_sizer = wx.BoxSizer(wx.VERTICAL)
        
        # Header
        header_sizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.lbl_plugin = wx.StaticText(self, label=f"Editing: {self.mapping.plugin_name}")
        font = self.lbl_plugin.GetFont()
        font.MakeBold()
        self.lbl_plugin.SetFont(font)
        header_sizer.Add(self.lbl_plugin, 1, wx.ALIGN_CENTER_VERTICAL)
        
        # Reload Dump Button
        self.btn_reload = wx.Button(self, label="Reload Dump")
        self.btn_reload.Bind(wx.EVT_BUTTON, self.on_reload_dump)
        header_sizer.Add(self.btn_reload, 0, wx.LEFT, 10)
        
        main_sizer.Add(header_sizer, 0, wx.EXPAND | wx.ALL, 10)
        
        content_sizer = wx.BoxSizer(wx.HORIZONTAL)
        
        # --- COL 1: CONTROLS (Knobs/Buttons) ---
        col1 = wx.BoxSizer(wx.VERTICAL)
        col1.Add(wx.StaticText(self, label="1. Control List (Select to Edit)"), 0, wx.BOTTOM, 5)
        
        # Control List (ListBox)
        self.lst_controls = wx.ListBox(self, style=wx.LB_SINGLE)
        self.lst_controls.Bind(wx.EVT_LISTBOX, self.on_control_select)
        col1.Add(self.lst_controls, 1, wx.EXPAND | wx.BOTTOM, 5)
        
        # Control Config Box (Under List 1)
        config_box = wx.StaticBoxSizer(wx.VERTICAL, self, label="Settings")
        
        # Shift Checkbox
        self.chk_shift = wx.CheckBox(self, label="Shift Layer")
        self.chk_shift.Bind(wx.EVT_CHECKBOX, self.on_shift_change)
        config_box.Add(self.chk_shift, 0, wx.ALL, 5)
        
        # Type Radio
        row_type = wx.BoxSizer(wx.HORIZONTAL)
        self.opt_rotate = wx.RadioButton(self, label="Rotate", style=wx.RB_GROUP)
        self.opt_touch = wx.RadioButton(self, label="Touch")
        self.opt_rotate.Bind(wx.EVT_RADIOBUTTON, self.on_type_change)
        self.opt_touch.Bind(wx.EVT_RADIOBUTTON, self.on_type_change)
        
        row_type.Add(self.opt_rotate, 0, wx.RIGHT, 10)
        row_type.Add(self.opt_touch, 0)
        config_box.Add(row_type, 0, wx.ALL, 5)
        
        col1.Add(config_box, 0, wx.EXPAND | wx.ALL, 5)
        content_sizer.Add(col1, 1, wx.EXPAND | wx.ALL, 10)
        
        # --- COL 2: PARAMETERS (Assign) ---
        col2 = wx.BoxSizer(wx.VERTICAL)
        col2.Add(wx.StaticText(self, label="2. Parameters (Enter to Assign)"), 0, wx.BOTTOM, 5)
        
        self.txt_search = wx.TextCtrl(self)
        self.txt_search.SetHint("Search...")
        self.txt_search.Bind(wx.EVT_TEXT, self.on_search)
        col2.Add(self.txt_search, 0, wx.EXPAND | wx.BOTTOM, 5)
        
        self.lst_source = wx.ListBox(self, style=wx.LB_SINGLE)
        self.lst_source.Bind(wx.EVT_LISTBOX_DCLICK, self.on_assign)
        self.lst_source.Bind(wx.EVT_KEY_DOWN, self.on_source_key)
        
        col2.Add(self.lst_source, 1, wx.EXPAND | wx.BOTTOM, 5)
        
        btn_assign = wx.Button(self, label="Assign (Enter)")
        btn_assign.Bind(wx.EVT_BUTTON, self.on_assign)
        col2.Add(btn_assign, 0, wx.EXPAND)
        
        content_sizer.Add(col2, 1, wx.EXPAND | wx.ALL, 10)
        
        # --- COL 3: PAGES ---
        col3 = wx.BoxSizer(wx.VERTICAL)
        col3.Add(wx.StaticText(self, label="3. Pages"), 0, wx.BOTTOM, 5)
        
        self.lst_pages = wx.ListBox(self, style=wx.LB_SINGLE)
        self.lst_pages.Bind(wx.EVT_LISTBOX, self.on_page_select)
        self.lst_pages.Bind(wx.EVT_CONTEXT_MENU, self.on_page_context) 
        col3.Add(self.lst_pages, 1, wx.EXPAND | wx.BOTTOM, 5)
        
        btn_add_page = wx.Button(self, label="Add Page")
        btn_add_page.Bind(wx.EVT_BUTTON, self.on_add_page)
        col3.Add(btn_add_page, 0, wx.EXPAND)
        
        content_sizer.Add(col3, 0, wx.EXPAND | wx.ALL, 10) # Less width for pages
        
        main_sizer.Add(content_sizer, 1, wx.EXPAND)
        
        # Footer
        footer_sizer = wx.BoxSizer(wx.HORIZONTAL)
        btn_back_main = wx.Button(self, label="< Back")
        btn_back_main.Bind(wx.EVT_BUTTON, lambda e: self.on_back())
        
        btn_save_all = wx.Button(self, label="Save Mapping")
        btn_save_all.Bind(wx.EVT_BUTTON, lambda e: self.on_save())
        
        footer_sizer.Add(btn_back_main, 0, wx.ALL, 5)
        footer_sizer.AddStretchSpacer()
        footer_sizer.Add(btn_save_all, 0, wx.ALL, 5)
        
        main_sizer.Add(footer_sizer, 0, wx.EXPAND | wx.ALL, 5)
        
        self.SetSizer(main_sizer)
        
        # Init Data
        self.refresh_source_list()
        self.refresh_page_list()     # Updates page list
        self.refresh_control_list()  # Updates knob/button list based on page
        
        # Set Tab Order for Accessibility
        self.lst_controls.MoveAfterInTabOrder(self.btn_reload)
        self.txt_search.MoveAfterInTabOrder(self.lst_controls)
        self.lst_source.MoveAfterInTabOrder(self.txt_search)
        self.lst_pages.MoveAfterInTabOrder(self.lst_source)
        
        # Accessibility: First focus on Control List
        wx.CallAfter(self.lst_controls.SetFocus)
        if self.lst_controls.GetCount() > 0:
            self.lst_controls.SetSelection(0)

    # --- LIST 1: CONTROLS ---
    
    def refresh_control_list(self):
        # Save current selection if possible
        sel = self.lst_controls.GetSelection()
        
        self.lst_controls.Clear()
        
        # 1. Knobs (1-8) for Current Page
        page_idx = self.lst_pages.GetSelection()
        if page_idx == wx.NOT_FOUND: page_idx = 0
        
        if page_idx < len(self.mapping.pages):
            page = self.mapping.pages[page_idx]
            for i in range(8):
                knob = page.knobs[i]
                # Determine what to show based on Shift/Touch
                is_shift = self.chk_shift.GetValue()
                is_touch = self.opt_touch.GetValue()
                
                label = f"Knob {i+1}"
                val = -1
                
                if is_touch:
                    label += " [Touch]"
                    val = knob.touch_param_id
                elif is_shift:
                    label += " [Shift]"
                    val = knob.shift_param_id
                else: 
                    # Default Rotate
                    val = knob.param_id
                    
                assigned_name = self.get_display_name(val)
                self.lst_controls.Append(f"{label}: {assigned_name}", i) # Data = Knob Index (0-7)

        # 2. Buttons (Always append at end)
        for b_name in MAPPABLE_BUTTONS:
             val = self.mapping.buttons.get(b_name, -1)
             assigned_name = self.get_display_name(val)
             self.lst_controls.Append(f"Button {b_name}: {assigned_name}", b_name) # Data = Button Name (str)
             
        # Restore selection
        if sel != wx.NOT_FOUND and sel < self.lst_controls.GetCount():
            self.lst_controls.SetSelection(sel)
        elif self.lst_controls.GetCount() > 0:
            self.lst_controls.SetSelection(0)
            
    def on_control_select(self, evt):
        # Update UI state (enable/disable Rotate/Touch)
        sel_idx = self.lst_controls.GetSelection()
        if sel_idx == wx.NOT_FOUND: return
        
        data = self.lst_controls.GetClientData(sel_idx)
        is_knob = isinstance(data, int) # Knob index is int, Button name is str
        
        self.opt_rotate.Enable(is_knob)
        self.opt_touch.Enable(is_knob)
        
        if not is_knob:
            self.opt_rotate.SetValue(True) # Default for button

    def on_shift_change(self, evt):
        self.refresh_control_list()
        
    def on_type_change(self, evt):
        self.refresh_control_list()

    # --- LIST 2: SOURCE PARAMS ---

    def on_source_key(self, evt):
        if evt.GetKeyCode() == wx.WXK_RETURN:
            self.on_assign(evt)
        else:
            evt.Skip()
    
    def on_reload_dump(self, evt):
        # Show paste dialog
        dlg = wx.TextEntryDialog(self, 
            "Paste the FX parameter dump from REAPER (@DUMP_FX):", 
            "Reload Parameter List",
            style=wx.OK | wx.CANCEL | wx.TE_MULTILINE)
        dlg.SetSize((600, 400))
        
        if dlg.ShowModal() == wx.ID_OK:
            text = dlg.GetValue().strip()
            if text:
                # Use MainFrame's parse_dump (which expects REAPER format)
                parent = self.GetParent()
                if hasattr(parent, 'parse_dump'):
                    # Store current plugin name and mappings
                    old_name = self.mapping.plugin_name
                    if parent.parse_dump(text):
                        # Restore plugin name if different (user might be reloading for same plugin)
                        # Actually, keep new name in case it's an update
                        # Refresh UI
                        self.refresh_source_list()
                        self.refresh_control_list()
                        wx.MessageBox(f"Loaded {len(self.mapping.params)} parameters", "Success")
                    else:
                        wx.MessageBox("Invalid dump format", "Error")
        
        dlg.Destroy()

    def refresh_source_list(self):
        query = self.txt_search.GetValue().lower()
        self.lst_source.Clear()
        
        # Special Actions
        for act_code, act_name in SPECIAL_ACTIONS.items():
            if not query or query in act_name.lower():
                self.lst_source.Append(f"{act_name} ({act_code})", act_code)
        
        # Params
        sorted_ids = sorted(self.mapping.params.keys())
        for pid in sorted_ids:
            name = self.mapping.params[pid]
            if not query or query in name.lower() or str(pid) in query:
                self.lst_source.Append(f"{pid}: {name}", pid)
                
    def on_search(self, evt):
        self.refresh_source_list()

    def on_assign(self, evt):
        # 1. Get Control Target
        sel_ctrl_idx = self.lst_controls.GetSelection()
        if sel_ctrl_idx == wx.NOT_FOUND:
            wx.MessageBox("Please select a Control (Knob/Button) first.", "Error")
            return
        
        target_data = self.lst_controls.GetClientData(sel_ctrl_idx) # Int (Knob) or Str (Button)
        
        # 2. Get Source Value
        sel_src_idx = self.lst_source.GetSelection()
        if sel_src_idx == wx.NOT_FOUND: return
        source_val = self.lst_source.GetClientData(sel_src_idx)
        
        # 3. Apply assignment
        if isinstance(target_data, int):
            # KNOB
            knob_idx = target_data
            page_idx = self.lst_pages.GetSelection()
            if page_idx == wx.NOT_FOUND: page_idx = 0
            
            knob = self.mapping.pages[page_idx].knobs[knob_idx]
            
            is_touch = self.opt_touch.GetValue()
            is_shift = self.chk_shift.GetValue()
            
            # Knob Safety: No Actions on Knobs yet
            if isinstance(source_val, str) and source_val.startswith("@"):
                 wx.Bell() 
                 return

            pid = int(source_val)
            if is_touch: knob.touch_param_id = pid
            elif is_shift: knob.shift_param_id = pid
            else: knob.param_id = pid
            
        else:
            # BUTTON
            b_name = target_data
            self.mapping.buttons[b_name] = source_val
            
        # 4. Refresh & Focus
        self.refresh_control_list()
        self.lst_controls.SetFocus()

    # --- LIST 3: PAGES ---
    
    def refresh_page_list(self):
        sel = self.lst_pages.GetSelection()
        self.lst_pages.Clear()
        for p in self.mapping.pages:
            self.lst_pages.Append(p.name)
        
        if sel != wx.NOT_FOUND and sel < len(self.mapping.pages):
            self.lst_pages.SetSelection(sel)
        else:
            self.lst_pages.SetSelection(0)

    def on_add_page(self, evt):
        self.mapping.pages.append(Page(f"Page {len(self.mapping.pages)+1}"))
        self.refresh_page_list()
        self.lst_pages.SetSelection(len(self.mapping.pages)-1)
        self.refresh_control_list()

    def on_page_select(self, evt):
        self.refresh_control_list()

    def on_page_context(self, evt):
        sel = self.lst_pages.GetSelection()
        if sel == wx.NOT_FOUND: return
        
        menu = wx.Menu()
        menu.Append(1, "Rename")
        menu.Append(2, "Delete")
        menu.AppendSeparator()
        menu.Append(3, "Move Up")
        menu.Append(4, "Move Down")
        
        menu.Bind(wx.EVT_MENU, self.on_page_rename, id=1)
        menu.Bind(wx.EVT_MENU, self.on_page_delete, id=2)
        menu.Bind(wx.EVT_MENU, self.on_page_move_up, id=3)
        menu.Bind(wx.EVT_MENU, self.on_page_move_down, id=4)
        
        self.PopupMenu(menu)
        menu.Destroy()

    def on_page_rename(self, evt):
        sel = self.lst_pages.GetSelection()
        page = self.mapping.pages[sel]
        dlg = wx.TextEntryDialog(self, "Rename Page:", "Rename", page.name)
        if dlg.ShowModal() == wx.ID_OK:
            page.name = dlg.GetValue()
            self.refresh_page_list()
            
    def on_page_delete(self, evt):
        sel = self.lst_pages.GetSelection()
        if len(self.mapping.pages) <= 1: return
        del self.mapping.pages[sel]
        self.refresh_page_list()
        self.refresh_control_list()
        
    def on_page_move_up(self, evt):
        sel = self.lst_pages.GetSelection()
        if sel > 0:
            self.mapping.pages[sel], self.mapping.pages[sel-1] = self.mapping.pages[sel-1], self.mapping.pages[sel]
            self.refresh_page_list()
            self.lst_pages.SetSelection(sel-1)
            self.refresh_control_list()
            
    def on_page_move_down(self, evt):
        sel = self.lst_pages.GetSelection()
        if sel < len(self.mapping.pages) - 1:
            self.mapping.pages[sel], self.mapping.pages[sel+1] = self.mapping.pages[sel+1], self.mapping.pages[sel]
            self.refresh_page_list()
            self.lst_pages.SetSelection(sel+1)
            self.refresh_control_list()

    def get_display_name(self, val):
        if val == -1 or val == "-1": return "-"
        if isinstance(val, str) and val.startswith("@"):
            return f"{SPECIAL_ACTIONS.get(val, val)}"
        try:
            pid = int(val)
            name = self.mapping.params.get(pid, '?')
            return f"[{pid}] {name}"
        except:
            return str(val)

class MainFrame(wx.Frame):
    def __init__(self):
        super().__init__(None, title="SoundFirst PRO Mapper", size=(1000, 700))
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)
        self.mapping = FXMapping()
        
        self.show_start()
        
    def show_start(self):
        self.clear_panel()
        self.panel = StartPanel(self, self.on_start_next)
        self.sizer.Add(self.panel, 1, wx.EXPAND)
        self.Layout()
        
    def on_start_next(self, mode):
        if mode == "new":
            self.show_dump()
        else:
            self.import_ini() # If successful, goes to editor
            
    def show_dump(self):
        self.clear_panel()
        self.panel = DumpPanel(self, self.on_dump_next, self.show_start)
        self.sizer.Add(self.panel, 1, wx.EXPAND)
        self.Layout()
        
    def on_dump_next(self, dump_text):
        if self.parse_dump(dump_text):
            self.show_editor()
            
    def show_editor(self):
        self.clear_panel()
        self.panel = EditorPanel(self, self.mapping, self.show_start, self.save_ini)
        self.sizer.Add(self.panel, 1, wx.EXPAND)
        self.Layout()
        
    def clear_panel(self):
        if hasattr(self, 'panel'):
            self.panel.Destroy()
            
    def parse_dump(self, text):
        lines = text.splitlines()
        plugin_name = "Unknown"
        params = {}
        valid_header = False
        
        for line in lines:
            line = line.strip()
            if not line: continue
            if line.startswith("[") and line.endswith("]"):
                plugin_name = line[1:-1]
                valid_header = True
                continue
            m = re.match(r'^;\s*(\d+)\s*=\s*(.*)', line)
            if m:
                params[int(m.group(1))] = m.group(2).strip()
        
        if not valid_header and not params:
            wx.MessageBox("Invalid Dump Format.", "Error")
            return False
            
        self.mapping.plugin_name = plugin_name
        self.mapping.params = params
        return True
            
    def import_ini(self):
        appdata = os.getenv("APPDATA")
        maps_dir = os.path.join(appdata, "REAPER", "UserPlugins", "SoundFirst_Maps")
        os.makedirs(maps_dir, exist_ok=True)
        
        dlg = wx.FileDialog(self, "Open Mapping", defaultDir=maps_dir, wildcard="INI files (*.ini)|*.ini",
                            style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST)
        if dlg.ShowModal() == wx.ID_CANCEL: return
        path = dlg.GetPath()
        
        config = configparser.ConfigParser()
        config.read(path)
        
        if 'Main' not in config:
            wx.MessageBox("Invalid mapping file", "Error")
            return
            
        self.mapping = FXMapping()
        self.mapping.plugin_name = config['Main'].get('PluginName', "Unknown")
        
        # Load Pages
        self.mapping.pages = []
        i = 1
        while f"Page{i}" in config:
            sec = config[f"Page{i}"]
            page = Page(sec.get('Name', f"Page {i}"))
            for k in range(8):
                knob = page.knobs[k]
                # Extract numeric part (before any ; comment)
                def parse_int(val_str, default=-1):
                    if not val_str or val_str == "-1": return default
                    val_str = val_str.split(';')[0].strip()  # Remove comment
                    try:
                        return int(val_str)
                    except ValueError:
                        return default
                
                knob.param_id = parse_int(sec.get(f"K{k+1}", "-1"))
                knob.shift_param_id = parse_int(sec.get(f"K{k+1}_Shift", "-1"))
                knob.touch_param_id = parse_int(sec.get(f"K{k+1}_Touch", "-1"))
            self.mapping.pages.append(page)
            i += 1
        if not self.mapping.pages: self.mapping.pages.append(Page())

        # Load Buttons
        if 'Buttons' in config:
            for b_name in MAPPABLE_BUTTONS:
                val_str = config['Buttons'].get(b_name, "-1")
                # Remove comment if present
                val_str = val_str.split(';')[0].strip()
                try:
                    self.mapping.buttons[b_name] = int(val_str)
                except ValueError:
                    self.mapping.buttons[b_name] = val_str
        
        # Note: Params are empty if we just import INI. 
        # Ideally user loads dump OR we just show IDs.
        # Fix: Show IDs in UI if params missing.
        
        self.show_editor()

    def save_ini(self):
        appdata = os.getenv("APPDATA")
        maps_dir = os.path.join(appdata, "REAPER", "UserPlugins", "SoundFirst_Maps")
        os.makedirs(maps_dir, exist_ok=True)

        # Sanitize filename (remove prefix and developer suffix)
        name = self.mapping.plugin_name
        # Remove Prefix VST:, JS:, etc.
        if ":" in name:
            name = name.split(":", 1)[1].strip()
        # Remove Developer Suffix (Name)
        if "(" in name and name.endswith(")"):
            name = name.rsplit("(", 1)[0].strip()
            
        safe_name = re.sub(r'[<>:"/\\|?*]', '_', name)
        
        dlg = wx.FileDialog(self, "Save Mapping", defaultDir=maps_dir,
                            defaultFile=f"{safe_name}.ini",
                            wildcard="INI files (*.ini)|*.ini",
                            style=wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
                            
        if dlg.ShowModal() == wx.ID_CANCEL: return
        path = dlg.GetPath()
        
        config = configparser.ConfigParser()
        
        config['Main'] = {'PluginName': self.mapping.plugin_name}
        
        for i, page in enumerate(self.mapping.pages):
            sec = f"Page{i+1}"
            config[sec] = {'Name': page.name}
            for k, knob in enumerate(page.knobs):
                # Helper to get formatted string "PID ; Name"
                def get_val_str(pid):
                    if pid == -1: return None
                    name = self.mapping.params.get(pid, "")
                    # Sanitize name for INI safety (remove ; or # if any)
                    safe_name = re.sub(r'[;#]', '', name).strip()
                    if safe_name: return f"{pid} ; {safe_name}"
                    return str(pid)

                if knob.param_id != -1: 
                    config[sec][f"K{k+1}"] = get_val_str(knob.param_id)
                if knob.shift_param_id != -1: 
                    config[sec][f"K{k+1}_Shift"] = get_val_str(knob.shift_param_id)
                if knob.touch_param_id != -1: 
                    config[sec][f"K{k+1}_Touch"] = get_val_str(knob.touch_param_id)
        
        config['Buttons'] = {}
        for b_name, val in self.mapping.buttons.items():
            if val != -1:
                # Buttons can be Int (PID) or Str (Action)
                s_val = str(val)
                comment = ""
                if isinstance(val, int):
                    name = self.mapping.params.get(val, "")
                    safe_name = re.sub(r'[;#]', '', name).strip()
                    if safe_name: comment = f" ; {safe_name}"
                elif isinstance(val, str) and val.startswith("@"):
                    # Add friendly name for action
                    friendly = SPECIAL_ACTIONS.get(val, "")
                    if friendly: comment = f" ; {friendly}"
                
                config['Buttons'][b_name] = f"{s_val}{comment}"
                
        with open(path, 'w') as f:
            config.write(f)
            
        wx.MessageBox(f"Mapping saved to {os.path.basename(path)}", "Success")

if __name__ == "__main__":
    app = wx.App(False)
    frame = MainFrame()
    frame.Show()
    app.MainLoop()

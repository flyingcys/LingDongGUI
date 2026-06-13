#!/usr/bin/env python3
"""
Analyze all non-static functions in tinyui/*.c, auto-fill doxygen headers:
- @brief: from function name and implementation pattern
- @param[in]/@param[out]: from const qualifier and pointer usage
- @return: from actual return statements in function body

Run from repo root: python3 scripts/fill_doxygen.py
"""

import re
import os
import sys
from collections import Counter

GREEN = "\033[32m"
YELLOW = "\033[33m"
RESET = "\033[0m"


# ── Helper: decode snake_case to words ──

def snake_to_words(s):
    """Convert snake_case to list of English words."""
    return s.split('_')


def capitalize_first(s):
    """Capitalize first char."""
    if not s:
        return s
    return s[0].upper() + s[1:]


# ── Function description templates ──

def describe_func_create(module, with_props=False):
    m = module.replace('_', ' ')
    if module in ('app', 'theme', 'native'):
        return 'Create %s instance' % m
    if with_props:
        return 'Create %s widget with properties' % m
    return 'Create %s widget' % m


def describe_func_destroy(module):
    m = module.replace('_', ' ')
    if module in ('app', 'theme', 'native'):
        return 'Destroy %s instance' % m
    return 'Destroy %s widget' % m


def describe_func_set(module, prop):
    m = module.replace('_', ' ')
    p = prop.replace('_', ' ')
    if module in ('app', 'theme', 'widget', 'window', 'native', 'event'):
        return 'Set %s of %s' % (p, m)
    return 'Set %s of %s widget' % (p, m)


def describe_func_get(module, prop):
    m = module.replace('_', ' ')
    p = prop.replace('_', ' ')
    if module in ('app', 'theme', 'widget', 'window', 'native', 'event'):
        return 'Get %s of %s' % (p, m)
    return 'Get %s of %s widget' % (p, m)


def describe_func_has(prop):
    return 'Check whether %s exists' % prop.replace('_', ' ')


def describe_func_is(prop):
    return 'Check whether %s is active' % prop.replace('_', ' ')


# ── Parser ──

def find_all_doxygen_starts(text):
    """Find all positions of '/**' (doxygen block starts)."""
    positions = []
    i = 0
    while i < len(text):
        if text[i:i+3] == '/**' and (i + 3 >= len(text) or text[i+3] != '*'):
            if i + 3 < len(text) and text[i+3] == '*':
                i += 3
                continue
            positions.append(i)
            i += 3
            continue
        i += 1
    return positions


def parse_function_def(text, pos):
    """Parse function definition starting at pos.
    Returns (func_name, params_str, ret_type, body_start, body_end) or None.
    """
    i = pos
    n = len(text)
    pd = 0
    found_paren = False
    sig_end = -1

    while i < n:
        ch = text[i]
        if ch == '"':
            i += 1
            while i < n and text[i] != '"':
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        if ch == "'":
            i += 1
            while i < n and text[i] != "'":
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        if ch == '/' and i + 1 < n and text[i + 1] == '/':
            while i < n and text[i] != '\n':
                i += 1
            i += 1
            continue
        if ch == '/' and i + 1 < n and text[i + 1] == '*':
            i += 2
            while i < n and not (text[i] == '*' and i + 1 < n and text[i + 1] == '/'):
                i += 1
            i += 2
            continue

        if ch == '(':
            found_paren = True
            pd += 1
        elif ch == ')':
            pd -= 1
        elif ch == '{' and pd == 0 and found_paren:
            sig_end = i
            break
        elif ch == ';' and pd == 0 and found_paren:
            return None

        i += 1

    if sig_end < 0:
        return None

    sig_text = text[pos:sig_end].strip()
    paren_pos = sig_text.find('(')
    if paren_pos < 0:
        return None

    func_name = ''
    j = paren_pos - 1
    while j >= 0 and (sig_text[j].isalnum() or sig_text[j] == '_'):
        j -= 1
    func_name = sig_text[j+1:paren_pos]
    if not func_name:
        return None

    # Extract params text between ( and )
    abs_paren_pos = pos + paren_pos
    pd2 = 0
    p_start = None
    params_str = ''
    for k in range(abs_paren_pos, min(abs_paren_pos + 2000, len(text))):
        if text[k] == '(' and pd2 == 0:
            p_start = k
            pd2 = 1
        elif text[k] == '(':
            pd2 += 1
        elif text[k] == ')' and pd2 == 1:
            params_str = text[p_start+1:k]
            break
        elif text[k] == ')':
            pd2 -= 1

    before_func = sig_text[:paren_pos].strip().rsplit(func_name, 1)[0].strip()
    ret_type = before_func if before_func else ''

    # Match braces for body
    body_start = sig_end + 1
    bd = 1
    body_end = body_start
    in_s = False
    in_c = False
    while body_end < n and bd > 0:
        ch = text[body_end]
        if ch == '"' and not in_c:
            in_s = not in_s
        if ch == '/' and body_end + 1 < n:
            if text[body_end + 1] == '/' and not in_s:
                in_c = True
                body_end += 1
                continue
            if text[body_end + 1] == '*' and not in_s:
                in_c = True
                body_end += 1
                continue
        if ch == '\n':
            in_c = False
        if not in_s and not in_c:
            if ch == '{':
                bd += 1
            elif ch == '}':
                bd -= 1
        body_end += 1

    return (func_name, params_str, ret_type, body_start, body_end)


def parse_params(params_str):
    """Parse param list string into list of dicts."""
    s = params_str.strip()
    if s == 'void' or not s:
        return []

    # Split by comma at depth 0
    parts = []
    depth = 0
    cur = ""
    for ch in s:
        if ch in '({[':
            depth += 1
            cur += ch
        elif ch in ')}]':
            depth -= 1
            cur += ch
        elif ch == ',' and depth == 0:
            parts.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur.strip())

    result = []
    for p in parts:
        p = re.sub(r'/\*.*?\*/', '', p).strip()
        if not p or p == 'void':
            continue

        is_const = bool(re.search(r'\bconst\b', p))
        tokens = re.findall(r'[*]+|[^*\s]+', p)
        merged = []
        for t in tokens:
            if merged and set(t) == {'*'}:
                merged[-1] = merged[-1] + t
            else:
                merged.append(t)

        type_kw = {
            "int", "char", "float", "double", "long", "short", "unsigned",
            "signed", "const", "volatile", "void", "struct", "enum", "union",
            "auto", "register", "extern", "inline", "restrict",
            "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
            "int32_t", "int64_t", "size_t", "ssize_t", "bool", "intptr_t",
            "uintptr_t", "uint_fast8_t", "uint_fast16_t", "uint_fast32_t",
            "uint_least8_t", "uint_least16_t", "uint_least32_t",
        }

        name = None
        for t in reversed(merged):
            tc = t.rstrip(';[],')
            if tc not in type_kw and not set(tc).issubset({'*'}):
                name = tc
                break

        if name:
            ptr_levels = 0
            for t in merged:
                if name in t:
                    ptr_levels += t.count('*')
                elif set(t) == {'*'}:
                    ptr_levels += len(t)
            result.append({
                'name': name,
                'raw': p,
                'is_const': is_const,
                'is_ptr': ptr_levels > 0,
            })

    return result


def infer_param_direction(param, body, func_name):
    """Infer [in] or [out] based on usage."""
    name = param['name']
    if param['is_const']:
        return 'in'
    if not param['is_ptr']:
        return 'in'

    out_prefixes = ('out_', 'p_')
    if any(name.startswith(p) for p in out_prefixes):
        return 'out'

    # Dereferenced for write: *name =
    if re.search(r'\*' + re.escape(name) + r'\s*=\s*[^=]', body):
        return 'out'

    # Passed to memcpy/memset etc as first arg
    out_fns = ['memcpy', 'memset', 'memmove', 'sprintf', 'snprintf', 'strcpy', 'strncpy']
    for fn in out_fns:
        pat = r'\b' + fn + r'\s*\(\s*' + re.escape(name)
        if re.search(pat, body):
            return 'out'

    # In get_ functions, non-const pointer is likely output
    if '_get_' in func_name:
        return 'out'
    if '_set_' in func_name:
        return 'in'

    return 'in'


# ── Description generators ──

def describe_function(func_name, body):
    """Generate @brief from function name and body."""

    # Handle special function names early
    special = {
        'main': 'Application entry point',
        'VT_enter_global_mutex': 'Enter global mutex lock',
        'VT_leave_global_mutex': 'Leave global mutex lock',
    }
    if func_name in special:
        return special[func_name]
    if func_name.startswith('ldCfg'):
        return 'Touch configuration: ' + func_name[6:]
    if func_name.startswith('xBtn'):
        return 'Backlight button handler'

    # Remove tinyui_ or tinyui_backend_ prefix
    base = func_name
    is_backend = False
    if base.startswith('tinyui_backend_'):
        base = base[15:]
        is_backend = True
    elif base.startswith('tinyui_'):
        base = base[7:]

    # Backend functions use backend description
    if is_backend:
        return describe_backend(base)

    # Non-backend detection
    if base.endswith('_create_with_props'):
        module = base[:-18]
        return describe_func_create(module, with_props=True)
    if base.endswith('_create'):
        module = base[:-7]
        return describe_func_create(module)
    if base.endswith('_destroy'):
        module = base[:-8]
        return describe_func_destroy(module)
    if '_set_' in base:
        parts = base.split('_set_', 1)
        return describe_func_set(parts[0], parts[1])
    if '_get_' in base:
        parts = base.split('_get_', 1)
        return describe_func_get(parts[0], parts[1])
    if base.startswith('has_'):
        return describe_func_has(base[4:])
    if base.startswith('is_'):
        return describe_func_is(base[3:])
    if base.startswith('app_'):
        return describe_app_action(base[4:])
    if base.startswith('widget_'):
        return describe_widget_action(base[7:])
    if base.startswith('window_'):
        return describe_window_action(base[7:])
    if base.startswith('theme_'):
        return describe_theme_action(base[6:])
    if base.startswith('event_'):
        return describe_event_action(base[6:])
    if base.startswith('native_'):
        return 'Native platform: ' + base[7:].replace('_', ' ')
    if base.startswith('image_source_'):
        rest = base[12:].strip('_')
        return 'Image source: ' + rest.replace('_', ' ')
    if base.startswith('font_'):
        return 'Font: ' + base[5:].replace('_', ' ')

    # Fallback
    return base.replace('_', ' ')


def describe_app_action(action):
    table = {
        'create': 'Create application instance',
        'destroy': 'Destroy application instance',
        'run': 'Run application event loop',
        'run_background': 'Run application in background mode',
        'set_window': 'Set application main window',
        'set_background': 'Set application background window',
        'switch_window': 'Switch application window',
        'switch_background': 'Switch application background window',
        'set_theme': 'Set application theme',
        'window_is_owned_by': 'Check if window belongs to this application',
    }
    return table.get(action, 'Application: ' + action.replace('_', ' '))


def describe_widget_action(action):
    table = {
        'claim_focus': 'Claim input focus',
        'find_by_id': 'Find widget by ID',
        'set_size': 'Set widget size',
        'set_grid_cell': 'Set widget grid position',
        'set_align': 'Set widget alignment',
        'destroy': 'Destroy widget',
        'attach_child': 'Attach child widget',
        'remove_child': 'Remove child widget',
        'set_event_handler': 'Set event handler',
        'set_pressed': 'Set pressed state',
        'get_root': 'Get root widget',
        'set_enable': 'Set enabled state',
        'set_focus': 'Set focus',
        'has_focus': 'Check if has focus',
    }
    for k, v in table.items():
        if action.startswith(k):
            return v
    if action:
        return 'Widget: ' + action.replace('_', ' ')
    return 'Widget operation'


def describe_window_action(action):
    table = {
        'create': 'Create window',
        'create_with_props': 'Create window with properties',
        'destroy': 'Destroy window',
        'set_color': 'Set window background color',
        'get_color': 'Get window background color',
        'set_background_source': 'Set window background source',
        'set_background_offset': 'Set window background offset',
        'get_background_offset': 'Get window background offset',
        'set_padding': 'Set window padding',
        'set_padding_group': 'Set window padding with grouping',
        'set_grid_padding': 'Set grid padding',
        'set_gap': 'Set grid gap',
        'set_layout_type': 'Set layout type',
        'get_padding_left': 'Get left padding',
        'get_padding_top': 'Get top padding',
        'get_padding_right': 'Get right padding',
        'get_padding_bottom': 'Get bottom padding',
    }
    return table.get(action, 'Window: ' + action.replace('_', ' '))


def describe_theme_action(action):
    table = {
        'create': 'Create theme instance',
        'destroy': 'Destroy theme instance',
    }
    return table.get(action, 'Theme: ' + action.replace('_', ' '))


def describe_event_action(action):
    table = {
        'register': 'Register event handler',
        'unregister': 'Unregister event',
        'process': 'Process event',
        'dispatch': 'Dispatch event',
    }
    return table.get(action, 'Event: ' + action.replace('_', ' '))


def describe_backend(base):
    """Describe backend function."""
    parts = base.split('_', 1)
    first = parts[0]
    rest = parts[1] if len(parts) > 1 else ''

    # Action comes first: backend_CREATE_MODULE or backend_SET_MODULE_PROP
    if first in ('create', 'destroy'):
        return capitalize_first(first) + ' backend for %s' % rest.replace('_', ' ')
    if first == 'draw':
        return 'Draw %s widget' % rest.replace('_', ' ')
    if first == 'layout':
        return 'Layout %s widget' % rest.replace('_', ' ')
    if first in ('init', 'run', 'shutdown'):
        return capitalize_first(first) + ' application backend'

    # Module comes first: backend_MODULE_create / backend_MODULE_set_PROP
    action_words = {
        'create': 'Create backend for %s widget',
        'destroy': 'Destroy backend for %s widget',
        'init': 'Initialize %s backend',
        'run': 'Run %s backend',
        'shutdown': 'Shutdown %s backend',
        'draw': 'Draw %s widget',
        'layout': 'Layout %s widget',
    }
    if rest in action_words:
        return action_words[rest] % first
    if rest.startswith('create_'):
        return 'Create %s widget backend' % rest[7:].replace('_', ' ')
    if rest.startswith('set_'):
        return 'Set %s of %s backend' % (rest[4:].replace('_', ' '), first)
    if rest.startswith('get_'):
        return 'Get %s from %s backend' % (rest[4:].replace('_', ' '), first)
    if rest.startswith('add_') or rest.startswith('remove_'):
        return capitalize_first(rest[0:4]) + ' %s from %s' % (rest[4:].replace('_', ' '), first)
    if rest.startswith('attach_') or rest.startswith('detach_'):
        return capitalize_first(rest[0:6]) + ' %s to %s' % (rest[6:].replace('_', ' '), first)
    if rest.startswith('has_') or rest.startswith('is_'):
        return 'Check %s of %s' % (rest.replace('_', ' '), first)
    if rest:
        return '%s: %s' % (first, rest.replace('_', ' '))
    return first


# ── Return description ──

def gen_return_desc(ret_type, body, func_name):
    """Generate @return text."""
    if not ret_type or ret_type.strip() == 'void':
        return None

    rt = ret_type.strip().rstrip('* ')

    # For .h declarations (no body), use type-based default
    if not body:
        if rt in ('void *',) or 'struct' in rt:
            return 'Pointer to the object on success, NULL on failure'
        if rt in ('int',) or rt.rstrip('* ') == 'int':
            if '_get_' in func_name:
                return 'The property value, negative on error'
            if func_name.startswith('has_') or func_name.startswith('is_'):
                return 'Non-zero if true, 0 otherwise'
            return '0 on success, -1 on failure'
        if rt == 'bool':
            return 'true if successful, false otherwise'
        return ''

    returns = re.findall(r'\breturn\s+([^;]+);', body)
    if not returns:
        return None

    # Count patterns
    has_null = any(r.strip() in ('NULL', '0', '(void*)0') for r in returns)
    has_nonnull = any(r.strip() not in ('NULL', '0', '(void*)0', '-1') for r in returns)
    has_minus1 = any(r.strip() == '-1' for r in returns)
    has_zero = any(r.strip() == '0' for r in returns)

    if rt in ('void *',) or 'struct' in rt:
        if has_null and has_nonnull:
            return 'Pointer to the object on success, NULL on failure'
        return 'Pointer to the object'

    if rt in ('int',) or rt.rstrip('* ') == 'int':
        if has_minus1 and has_zero:
            return '0 on success, -1 on failure'
        if has_zero and not has_minus1:
            return '0 on success'
        if has_minus1:
            return '-1 on failure'
        if '_get_' in func_name:
            return 'The property value, negative on error'
        if func_name.startswith('has_') or func_name.startswith('is_'):
            return 'Non-zero if true, 0 otherwise'
        return '0 on success, -1 on failure'

    if rt == 'bool':
        return 'true if successful, false otherwise'

    return ''


# ── Param description ──

PARAM_ENGLISH = {
    'parent': 'Parent widget',
    'id': 'Widget identifier string',
    'app': 'Application instance',
    'window': 'Window instance',
    'widget': 'Widget instance',
    'checkbox': 'Checkbox widget instance',
    'button': 'Button widget instance',
    'label': 'Label widget instance',
    'text': 'Text widget instance',
    'slider': 'Slider widget instance',
    'switch': 'Switch widget instance',
    'calendar': 'Calendar widget instance',
    'clock': 'Clock widget instance',
    'gauge': 'Gauge widget instance',
    'graph': 'Graph widget instance',
    'animation': 'Animation widget instance',
    'keyboard': 'Keyboard widget instance',
    'canvas': 'Canvas widget instance',
    'arc': 'Arc widget instance',
    'line_edit': 'Line edit widget instance',
    'list': 'List widget instance',
    'combo_box': 'Combo box widget instance',
    'date_time': 'Date time widget instance',
    'message_box': 'Message box widget instance',
    'radial_menu': 'Radial menu widget instance',
    'scroll_selecter': 'Scroll selecter widget instance',
    'qrcode': 'QR code widget instance',
    'progress_wheel': 'Progress wheel widget instance',
    'progress_bar': 'Progress bar widget instance',
    'icon_slider': 'Icon slider widget instance',
    'background': 'Background widget instance',
    'image': 'Image widget instance',
    'theme': 'Theme instance',
    'source': 'Image source',
    'props': 'Properties structure',
    'rgb': 'RGB color value (0xRRGGBB)',
    'bg_color': 'Background color',
    'fg_color': 'Foreground color',
    'text_color': 'Text color',
    'width': 'Width in pixels',
    'height': 'Height in pixels',
    'x': 'X coordinate',
    'y': 'Y coordinate',
    'offset_x': 'X offset',
    'offset_y': 'Y offset',
    'angle': 'Angle in degrees',
    'bg_start_angle': 'Background start angle',
    'bg_end_angle': 'Background end angle',
    'fg_end_angle': 'Foreground end angle',
    'rotation_angle': 'Rotation angle',
    'value': 'Value',
    'min': 'Minimum value',
    'max': 'Maximum value',
    'mode': 'Mode',
    'duration_ms': 'Duration in milliseconds',
    'period_ms': 'Period in milliseconds',
    'frame_index': 'Frame index',
    'row': 'Row index',
    'column': 'Column index',
    'row_span': 'Row span count',
    'col_span': 'Column span count',
    'gap': 'Gap in pixels',
    'left': 'Left padding',
    'top': 'Top padding',
    'right': 'Right padding',
    'bottom': 'Bottom padding',
    'space': 'Spacing',
    'radio_group': 'Radio button group ID',
    'type': 'Type',
    'format': 'Format string',
    'scale': 'Scale factor',
    'opacity': 'Opacity (0-255)',
    'radius': 'Radius',
    'thickness': 'Thickness',
    'ratio': 'Ratio',
    'enabled': 'Enable state',
    'visible': 'Visibility state',
    'state': 'State value',
    'buffer': 'Buffer pointer',
    'len': 'Length',
    'length': 'Length',
    'count': 'Count',
    'index': 'Index',
    'data': 'Data pointer',
    'user_data': 'User data pointer',
    'target': 'Target widget',
    'event': 'Event object',
    'msg': 'Message',
    'handle': 'Handle',
    'addr': 'Address',
    'offset': 'Offset',
    'point': 'Point',
    'pressed': 'Pressed state',
    'checked': 'Checked state',
    'img_tile': 'Image tile',
    'mask_tile': 'Mask tile',
    'header_format': 'Header format string',
    'show_header': 'Show header flag',
    'out': 'Output parameter',
    'mode': 'Operation mode',
    'name_id': 'Name identifier ID',
    'name': 'Name',
    'flag': 'Flag bits',
    'owner': 'Owner widget',
    'child': 'Child widget',
    'sort': 'Sort mode',
    'func': 'Function pointer',
    'arg': 'Argument',
    'size': 'Size in bytes',
    'addr': 'Address',
    'group': 'Group ID',
    'period_ms': 'Period in milliseconds',
    'color': 'Color value (0xRRGGBB)',
    'offset_x': 'Horizontal offset',
    'offset_y': 'Vertical offset',
    'bg_start_angle': 'Background arc start angle',
    'bg_end_angle': 'Background arc end angle',
    'fg_end_angle': 'Foreground arc end angle',
    'rotation_angle': 'Arc rotation angle',
}

# Add plural forms
PARAM_ENGLISH.update({
    'rows': 'Row definitions',
    'columns': 'Column definitions',
    'cols': 'Column definitions',
})


def gen_param_desc(param, func_name):
    """Generate param description."""
    name = param['name']
    base = re.sub(r'_[0-9]+$', '', name)
    if name in PARAM_ENGLISH:
        return PARAM_ENGLISH[name]
    if base in PARAM_ENGLISH:
        return PARAM_ENGLISH[base]
    # Try to make a readable description from the name
    return name.replace('_', ' ')


# ── Doxygen update ──

def update_doxygen_text(doxy_text, brief, params, ret_desc):
    """Rebuild doxygen comment with new descriptions."""
    new_lines = ['/**']
    new_lines.append(' * @brief ' + brief)
    new_lines.append(' *')
    for p in params:
        new_lines.append(' * @param[%s] %s %s' % (p['direction'], p['name'], p['desc']))
    if ret_desc:
        new_lines.append(' * @return %s' % ret_desc)
    new_lines.append(' */')
    return '\n'.join(new_lines)


# ── File processor ──

def parse_functions_from_blocks(filepath):
    """Parse file (.c), return list of function blocks and file text."""
    is_header = filepath.endswith('.h')
    if is_header:
        return parse_declarations_from_blocks(filepath)

    with open(filepath, 'r', encoding='utf-8') as f:
        text = f.read()

    functions = []
    starts = find_all_doxygen_starts(text)

    for ds in starts:
        de = text.find('*/', ds) + 2
        if de <= 2:
            continue
        doxy_text = text[ds:de]

        # Skip past blank lines
        pos = de
        while pos < len(text) and text[pos] in ' \t\n\r':
            pos += 1
        if pos >= len(text):
            continue

        info = parse_function_def(text, pos)
        if info:
            func_name, params_str, ret_type, body_start, body_end = info
            line_num = text[:pos].count('\n') + 1
            functions.append({
                'doxy_start': ds,
                'doxy_end': de,
                'doxy_text': doxy_text,
                'func_name': func_name,
                'params_str': params_str,
                'ret_type': ret_type,
                'body_text': text[body_start:body_end] if body_end > body_start else '',
                'line_num': line_num,
            })

    return functions, text


def find_all_decls_h(text):
    """Find all function declaration (;) positions in .h text, paired with preceding
    positions of doxygen blocks. Returns list of (decl_pos, doxy_start, doxy_end)."""
    result = []

    # Find all doxygen blocks
    starts = find_all_doxygen_starts(text)
    for ds in starts:
        de = text.find('*/', ds) + 2
        if de <= 2:
            continue

        # Skip blanks
        pos = de
        while pos < len(text) and text[pos] in ' \t\n\r':
            pos += 1
        if pos >= len(text):
            continue

        # Check if what follows is a function declaration
        info = parse_function_decl(text, pos)
        if info:
            func_name, params_str, ret_type, semi_pos = info
            line_num = text[:pos].count('\n') + 1
            result.append({
                'doxy_start': ds,
                'doxy_end': de,
                'doxy_text': text[ds:de],
                'func_name': func_name,
                'params_str': params_str,
                'ret_type': ret_type,
                'body_text': '',
                'line_num': line_num,
                'decl_pos': semi_pos,
            })

    return result


def parse_function_decl(text, pos):
    """Parse function declaration (prototype) in .h starting at pos.
    Declaration ends with ';'.
    Returns (func_name, params_str, ret_type, semi_pos) or None."""
    i = pos
    n = len(text)
    pd = 0
    found_paren = False

    while i < n:
        ch = text[i]
        # Skip strings
        if ch == '"':
            i += 1
            while i < n and text[i] != '"':
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        if ch == "'":
            i += 1
            while i < n and text[i] != "'":
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        if ch == '/' and i + 1 < n and text[i + 1] == '/':
            while i < n and text[i] != '\n':
                i += 1
            i += 1
            continue
        if ch == '/' and i + 1 < n and text[i + 1] == '*':
            i += 2
            while i < n and not (text[i] == '*' and i + 1 < n and text[i + 1] == '/'):
                i += 1
            i += 2
            continue

        if ch == '(':
            found_paren = True
            pd += 1
        elif ch == ')':
            pd -= 1
        elif ch == ';' and pd == 0 and found_paren:
            # Found end of declaration
            sig_text = text[pos:i].strip()
            paren_pos = sig_text.find('(')
            if paren_pos < 0:
                return None

            # Extract function name
            j = paren_pos - 1
            func_name = ''
            while j >= 0 and (sig_text[j].isalnum() or sig_text[j] == '_'):
                j -= 1
            func_name = sig_text[j + 1:paren_pos]
            if not func_name:
                return None

            # Check keywords
            keywords = {'if', 'for', 'while', 'switch', 'sizeof', 'return', 'catch', 'else', 'do'}
            if func_name in keywords:
                return None
            if func_name in ('uint8_t', 'uint16_t', 'uint32_t', 'int8_t', 'int16_t', 'int32_t', 'size_t'):
                return None

            # Return type
            before_func = sig_text[:paren_pos].strip().rsplit(func_name, 1)[0].strip()
            ret_type = before_func if before_func else ''

            # Extract params
            abs_paren = pos + paren_pos
            pd2 = 0
            p_start = None
            params_str = ''
            for k in range(abs_paren, min(abs_paren + 2000, n)):
                if text[k] == '(' and pd2 == 0:
                    p_start = k
                    pd2 = 1
                elif text[k] == '(':
                    pd2 += 1
                elif text[k] == ')' and pd2 == 1:
                    params_str = text[p_start + 1:k]
                    break
                elif text[k] == ')':
                    pd2 -= 1

            return (func_name, params_str, ret_type, i)

        elif ch == '{' and pd == 0:
            # Found body — this is a definition, not declaration
            return None
        elif ch == '=' and pd == 0:
            # Assignment (e.g., function pointer initialization)
            return None

        i += 1

    return None


def parse_declarations_from_blocks(filepath):
    """Parse .h file, return list of function declaration blocks."""
    with open(filepath, 'r', encoding='utf-8') as f:
        text = f.read()

    functions = []
    decls = find_all_decls_h(text)

    for d in decls:
        functions.append(d)

    return functions, text


def process_file(filepath):
    """Process one file: fill doxygen for all non-static functions."""
    functions, text = parse_functions_from_blocks(filepath)
    if not functions:
        return False

    # Sort descending by position for safe in-place replacement
    functions.sort(key=lambda f: f['doxy_start'], reverse=True)

    modified = False
    for func in functions:
        # Analyze
        params = parse_params(func['params_str'])
        body = func['body_text']
        brief = describe_function(func['func_name'], body)
        ret_desc = gen_return_desc(func['ret_type'], body, func['func_name'])

        # Infer param directions & get descriptions
        for p in params:
            p['direction'] = infer_param_direction(p, body, func['func_name'])
            p['desc'] = gen_param_desc(p, func['func_name'])

        # Build new doxygen
        new_doxy = update_doxygen_text(func['doxy_text'], brief, params, ret_desc)

        # Replace in text
        old_len = func['doxy_end'] - func['doxy_start']
        text = text[:func['doxy_start']] + new_doxy + text[func['doxy_end']:]

        relpath = os.path.relpath(filepath)
        print(f"  {GREEN}[FILL]{RESET} {relpath}:{func['line_num']} {func['func_name']}()")
        modified = True

    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(text)

    return modified


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    os.chdir(project_root)

    all_files = []
    for root_dir, dirs, files in os.walk('tinyui'):
        dirs[:] = [d for d in dirs if not d.startswith('.')]
        for f in files:
            if f.endswith('.c') or f.endswith('.h'):
                all_files.append(os.path.join(root_dir, f))
    all_files.sort()

    updated = 0
    for fp in all_files:
        try:
            if process_file(fp):
                updated += 1
        except Exception as e:
            print(f"  {RESET}[ERR]{RESET} {fp}: {e}", file=sys.stderr)

    print(f"\nDone. {updated} file(s) updated.")


if __name__ == '__main__':
    main()

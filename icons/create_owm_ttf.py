#!/usr/bin/env python3

import fontforge
import psMat
import os
import math
from fontTools.ttLib import TTFont
import re
import subprocess

name2unicode = {
}

def print_bounds(name,bounds,new_bounds):
    orig_width = round(bounds[2] - bounds[0])
    new_width = round(new_bounds[2] - new_bounds[0])
    orig_height = round(bounds[3] - bounds[1])
    new_height = round(new_bounds[3] - new_bounds[1])
    orig_xmin = round(bounds[0])
    orig_xmax = round(bounds[2])
    orig_ymin = round(bounds[1])
    orig_ymax = round(bounds[3])
    new_xmin = round(new_bounds[0])
    new_xmax = round(new_bounds[2])
    new_ymin = round(new_bounds[1])
    new_ymax = round(new_bounds[3])
    center_x = round(new_xmin + (new_xmax - new_xmin) / 2)
    center_y = round(new_ymin + (new_ymax - new_ymin) / 2)

    print(f'{name}: size {new_width}x{new_height} center {center_x},{center_y}')
    if orig_width != new_width or orig_height != new_height:
        print(f'  {orig_width}x{orig_height} -> {new_width}x{new_height}')
    if orig_xmin != new_xmin:
        print(f'  xmin {orig_xmin} -> {new_xmin}')
    if orig_xmax != new_xmax:
        print(f'  xmax {orig_xmax} -> {new_xmax}')
    if orig_ymin != new_ymin:
        print(f'  ymin {orig_ymin} -> {new_ymin}')
    if orig_ymax != new_ymax:
        print(f'  ymax {orig_ymax} -> {new_ymax}')
    print('')

def clean_raw_svg_text(svg_path):
    """
    Reads the raw SVG code, strips out the Google Material background box,
    and permanently saves the cleaned SVG file to the output folder.
    """
    if not os.path.exists(svg_path):
        print(f'Error: {svg_path} doesn\'t exist')
        return None

    with open(svg_path, 'r', encoding='utf-8') as f:
        content = f.read()

    patterns = [
        r'<path[^>]*fill=["\']none["\'][^>]*/>',
        r'<path[^>]*d=["\']M0\s+0h24v24H0z["\'][^>]*/>',
        r'<path[^>]*d=["\']M0\s+0h24v24h-24z["\'][^>]*/>',
        r'<rect[^>]*fill=["\']none["\'][^>]*/>',
        r'<rect[^>]*width=["\'](?:24|48)["\'][^>]*/>'
    ]

    cleaned_content = content
    for pattern in patterns:
        cleaned_content = re.sub(pattern, '', cleaned_content, flags=re.IGNORECASE)

    temp_file = 'cleaned_svg.svg'
    with open(temp_file, 'w', encoding='utf-8') as f:
        f.write(cleaned_content)
    return temp_file

def generate_html_mapping_sheet(mapping_data, ttf_filename, html_filename):
    """
    Generates a beautifully styled standalone HTML layout map showcasing 
    all processed icons, their names, and hex codes.
    """
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Font Layout Mapping Sheet</title>
    <style>
        @font-face {{
            font-family: 'MaterialCustomIcons';
            src: url('{os.path.basename(ttf_filename)}') format('truetype');
        }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: #f4f5f7;
            color: #333;
            padding: 40px;
        }}
        h1 {{ text-align: center; margin-bottom: 5px; color: #111; }}
        .subtitle {{ text-align: center; color: #666; margin-bottom: 40px; }}
        .grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));
            gap: 20px;
            max-width: 1200px;
            margin: 0 auto;
        }}
        .card {{
            background: #fff;
            border-radius: 8px;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0,0,0,0.04);
            transition: transform 0.2s, box-shadow 0.2s;
        }}
        .card:hover {{
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.08);
        }}
        .icon {{
            font-family: 'MaterialCustomIcons';
            font-size: 48px;
            color: black;
            margin-bottom: 15px;
            height: 50px;
            line-height: 50px;
        }}
        .name,.hex {{
            font-size: 14px;
            font-weight: 600;
            word-break: break-word;
            margin-bottom: 5px;
            color: #222;
        }}
    </style>
</head>
<body>

    <h1>Layout Mapping Sheet for """
    html_content += f"""{os.path.basename(ttf_filename)}</h1>
    <p class="subtitle">Generated dynamically from FontForge project</p>

    <div class="grid">
"""
    for name, hex_code in mapping_data:
        # Convert hex string safely to a browser-readable HTML entity reference
        html_entity = f"&#x{hex_code[2:].upper()};"
        html_content += f"""<div class="card">
            <div class="icon">{html_entity}</div>
            <div class="name">{name}</div>
            <div class="hex">{hex_code.upper()}</div>
        </div>\n"""

    html_content += """    </div>
</body>
</html>"""

    with open(html_filename, 'w', encoding='utf-8') as f:
        f.write(html_content)



def rotate_glyph(glyph, angle):
    # 1. Get the bounding box to calculate the exact center coordinates
    # boundingBox() returns (xmin, ymin, xmax, ymax)
    bbox = glyph.boundingBox()
    cx = (bbox[0] + bbox[2]) / 2
    cy = (bbox[1] + bbox[3]) / 2

    # 2. Define the transformation sequence
    # Move center to origin -> Rotate -> Move back from origin
    move_to_origin = psMat.translate(-cx, -cy)
    rotation_mat = psMat.rotate(math.radians(angle))
    move_back = psMat.translate(cx, cy)

    # 3. Chain the matrices together in reverse execution order
    final_matrix = psMat.compose(move_to_origin, rotation_mat)
    final_matrix = psMat.compose(final_matrix, move_back)

    # 4. Apply the transformation to the glyph
    glyph.transform(final_matrix)


def create_ttf_from_svg():
    # 1. Create a new font container
    font = fontforge.font()
    
    # 2. Set font metadata
    font.fontname = "OwmIcons"
    font.fullname = font.fontname
    font.familyname = font.fontname
    
    glyphs_to_add = {
        0xe63e: "wifi", # nb: "wifi" must come before wifi_*
        0xe4ca: "wifi_1_bar",
        0xe4d9: "wifi_2_bar",
        0xebe1: "wifi_3_bar",
        0xf0f0: "wifi_x",
        0xf0f1: "air_filter",
        0xf0fa: "error_icon",
        0xf0fb: "house_humidity",
        0xf0fc: "house_thermometer",
        0xf10e: "biological_hazard_symbol",
        0xf10f: "ionizing_radiation_symbol",
        0xf110: "warning_icon",
        0xe8f4: "visibility_icon",
        0xebdc: "battery_0_bar",
        0xebd9: "battery_1_bar",
        0xebe0: "battery_2_bar",
        0xebdd: "battery_3_bar",
        0xebe2: "battery_4_bar",
        0xebd4: "battery_5_bar",
        0xebd2: "battery_6_bar",
        0xe1a4: "battery_full",
        0xf0fe: "wind_direction_meteorological_0deg",
        0xf105: "wind_direction_meteorological_22_5deg",
        0xf10b: "wind_direction_meteorological_45deg",
        0xf10c: "wind_direction_meteorological_67_5deg",
        0xf10d: "wind_direction_meteorological_90deg",
        0xf0ff: "wind_direction_meteorological_112_5deg",
        0xf100: "wind_direction_meteorological_135deg",
        0xf101: "wind_direction_meteorological_157_5deg",
        0xf102: "wind_direction_meteorological_180deg",
        0xf103: "wind_direction_meteorological_202_5deg",
        0xf104: "wind_direction_meteorological_225deg",
        0xf106: "wind_direction_meteorological_247_5deg",
        0xf107: "wind_direction_meteorological_270deg",
        0xf108: "wind_direction_meteorological_292_5deg",
        0xf109: "wind_direction_meteorological_315deg",
        0xf10a: "wind_direction_meteorological_337_5deg",
        0xf111: "tide_down_arrow_water",
        0xf112: "tide_up_arrow_water",
    }

    max_width = 0
    max_height = 0
    mapping_records = []
    unitsPerEm = 1000
    wifi_shift_y = 0

    for unicode_dec, name in glyphs_to_add.items():
        # Create a glyph slot using the Unicode value
        glyph = font.createChar(unicode_dec)
        # Import the SVG vector outlines into the slot
        svg_path = 'svg/' + name + '.svg'
        if name.startswith('wind_direction_meteorological_'):
            svg_path = 'online_web_fonts/wind_direction.svg'
        if not os.path.exists(svg_path):
            svg_path = 'material_svg/' + name + '.svg'
        if not os.path.exists(svg_path):
            svg_path = 'online_web_fonts/' + name + '.svg'

        temp_file = clean_raw_svg_text(svg_path)
        glyph.importOutlines(temp_file)
        os.unlink(temp_file)

        if name.startswith('battery_'):
            rotate_glyph(glyph,-90)
            name = name + '_90deg'

        if name.startswith('wind_direction_meteorological_'):
            angle = 0.0
            deg_re = re.compile(r'.*_(\d{1,3})_(\d{1})deg')
            match = deg_re.match(name)
            if match:
                angle = float(match.group(1))
                angle += float(match.group(2)) / 10
            else:
                deg_re = re.compile(r'.*_(\d{1,3})deg')
                match = deg_re.match(name)
                if match:
                    angle = float(match.group(1))
            rotate_glyph(glyph,-angle)

        glyph.glyphname = name
        mapping_records.append((name, hex(unicode_dec)))

        #glyph.transform(offset_matrix)
        # 3. Determine the bounding box of the imported SVG 
        # (returns (xmin, ymin, xmax, ymax))
        bounds = glyph.boundingBox()

        # 4. Calculate the SVG's current center
        width = bounds[2] - bounds[0]
        height = bounds[3] - bounds[1]
        center_x = bounds[0] + (width / 2.0)
        center_y = bounds[1] + (height / 2.0)

        # 5. Determine where you want the shape centered in your em-square
        target_center_x = unitsPerEm/2
        target_center_y = unitsPerEm/2

        # 6. Calculate the X/Y shift needed
        shift_x = target_center_x - center_x
        shift_y = target_center_y - center_y
        if name.startswith('wifi'):
        # we want wifi_* to align with the full wifi icon "wifi" so
        # the dot at the bottom doen't move
            if name == 'wifi':
                wifi_shift_y = shift_y
            else:
                shift_y = wifi_shift_y

        #print(f'{name}: shift_x {shift_x} shift_y {shift_y}')
        # 7. Apply the transformation and clean up overlaps
        glyph.transform((1, 0, 0, 1, shift_x, shift_y))
        glyph.removeOverlap()

        new_bounds = glyph.boundingBox()
        #print_bounds(name,bounds,new_bounds)
        width = round(bounds[2] - bounds[0])
        height = round(bounds[3] - bounds[1])
        #left_side_bearing = unitsPerEm - ((bounds[2] - bounds[0]) // 2.0)
        #if 'wind_direction_meteorological' in name:
        #    glyph.left_side_bearing = 250
        #else:
        #    glyph.left_side_bearing = 0
        #
        ## must reset width (advancewidth) since setting left_side_bearing changes it
        glyph.width = unitsPerEm
        #print(f'{name}: advancewidth {glyph.width}')

        if max_width < width:
            max_width = width
        if max_height < height:
            max_height = height

        glyph.correctDirection()
        glyph.round()

    #print(f'max bounding box size {max_width}x{max_height}')
    # 4. Generate and save the final TTF file
    font.round()
    output_dir = '../data'
    output_ttf_name = f'{output_dir}/owm_icons.ttf'
    font.copyright = "Copyright: various see https://github.com/skiphansen/libDrawOWM/LICENSE"
    font.generate(output_ttf_name)
    print(f"{output_ttf_name} generated successfully!")
    map_name = f'{output_dir}/owm_icons.html'
    generate_html_mapping_sheet(mapping_records, output_ttf_name,map_name)
    shell = os.environ.get('SHELL')
    cmd_line = [ f'{shell}', '-c',f'ttx -f {output_ttf_name}']
    subprocess.run(cmd_line,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)

if __name__ == "__main__":
    create_ttf_from_svg()


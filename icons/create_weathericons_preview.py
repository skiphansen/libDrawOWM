#!/usr/bin/env python3

import html
import os
from fontTools.ttLib import TTFont


def get_font_glyphs(font_path):
    """Parses a TTF file and extracts valid Unicode code points and glyph names."""
    try:
        font = TTFont(font_path)
    except Exception as e:
        print(f"Error loading font file: {e}")
        return []

    # Get the best unicode character map available in the font
    cmap = font.getBestCmap()
    if not cmap:
        print("No valid Unicode character map found in this font.")
        return []

    glyphs_data = []
    # Sort by character code point order
    for code_point, glyph_name in sorted(cmap.items()):
        # Convert code point to actual character and its hex string representation
        char = chr(code_point)
        hex_string = f"U+{code_point:04X}"

        # Filter out problematic control characters that disrupt HTML rendering
        if code_point < 32 or (127 <= code_point <= 159):
            continue

        glyphs_data.append(
            {"char": char, "hex": hex_string, "name": glyph_name}
        )

    font.close()
    return glyphs_data

def generate_html(font_path, output_html_path="font_preview.html"):
    """Generates a responsive HTML page embedding the font and displaying its glyphs

    with bounding boxes representing the point size limits.
    """
    # Ensure the font file exists
    if not os.path.exists(font_path):
        print(f"Font file not found at: {font_path}")
        return

    # Extract font metadata for styling and file referencing
    font_filename = os.path.basename(font_path)
    font_name = os.path.splitext(font_filename)[0]

    print(f"Reading glyph data from {font_filename}...")
    glyphs = get_font_glyphs(font_path)

    if not glyphs:
        print("No renderable glyphs found. Aborting HTML generation.")
        return

    # Create the HTML structure with embedded CSS grid styling
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Glyph Preview: {html.escape(font_name)}</title>
    <style>
        @font-face {{
            font-family: 'CustomFont';
            src: url('{html.escape(font_path)}');
        }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: #f8f9fa;
            color: #212529;
            margin: 0;
            padding: 20px;
        }}
        header {{
            margin-bottom: 30px;
            border-bottom: 2px solid #dee2e6;
            padding-bottom: 15px;
        }}
        h1 {{ margin: 0 0 5px 0; font-size: 2rem; color: #343a40; }}
        .meta {{ color: #6c757d; font-size: 0.95rem; }}
        .grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
            gap: 15px;
        }}
        .card {{
            background: #ffffff;
            border: 1px solid #dee2e6;
            border-radius: 6px;
            padding: 15px 10px;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0,0,0,0.02);
            transition: transform 0.15s ease, box-shadow 0.15s ease;
        }}
        .card:hover {{
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.08);
            border-color: #b1b5b9;
        }}
        
        /* The Point Size Box Boundary Container */
        .glyph-container {{
            display: inline-flex;
            align-items: center;
            justify-content: center;
            position: relative;
            width: 90px;
            height: 90px; 
            margin: 0 auto 15px auto;
            border: 1px dashed #ff4d4d; /* Red Dashed Point Size Box */
            background-color: #fffafb;
        }}
        
        /* The actual character being rendered */
        .glyph {{
            font-family: 'CustomFont', sans-serif;
            font-size: 60px; /* Sized to sit inside the 90px point size container */
            line-height: 1;
            color: #000000;
            z-index: 2;
        }}
        
        /* Visual baseline indicator line */
        .baseline {{
            position: absolute;
            bottom: 22%; /* Standard structural baseline approximation */
            left: 0;
            right: 0;
            border-bottom: 1px dashed #4d94ff; /* Blue Dashed Baseline */
            z-index: 1;
        }}
        
        .code {{
            font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, monospace;
            font-size: 0.8rem;
            color: #495057;
            background: #e9ecef;
            padding: 2px 6px;
            border-radius: 4px;
            display: inline-block;
            margin-bottom: 4px;
        }}
        .name {{
            font-size: 0.75rem;
            color: #6c757d;
            word-break: break-all;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            padding: 0 4px;
        }}
    </style>
</head>
<body>

<header>
    <h1>Font Glyph Preview</h1>
    <div class="meta"><strong>Font File:</strong> {html.escape(font_filename)} | <strong>Total Glyphs:</strong> {len(glyphs)}</div>
</header>

<div class="grid">
"""

    # Populate the layout grid with dynamically processed glyph cards
    for item in glyphs:
        safe_char = html.escape(item["char"])
        safe_hex = html.escape(item["hex"])
        safe_name = html.escape(item["name"])

        html_content += f"""    <div class="card">
        <div class="glyph-container">
            <div class="baseline"></div>
            <div class="glyph">{safe_char}</div>
        </div>
        <div class="code">{safe_hex}</div>
        <div class="name" title="{safe_name}">{safe_name}</div>
    </div>\n"""

    # Close HTML structure tags
    html_content += """</div>

</body>
</html>
"""

    # Write the compiled markup out to the final file
    with open(output_html_path, "w", encoding="utf-8") as f:
        f.write(html_content)

    print(f"Successfully generated visual preview sheet at: {output_html_path}")


if __name__ == "__main__":
    # REPLACE THIS with the actual relative or absolute path to your local TTF file
    FONT_FILE = "../data/weathericons-regular-webfont.ttf"

    # Runs the generation routine
    generate_html(FONT_FILE, output_html_path="glyph_preview.html")


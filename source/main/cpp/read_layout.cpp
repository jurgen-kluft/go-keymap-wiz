#include "xjsmn/x_jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KLE_STRING_MAX_LENGTH 31
#define KLE_LABEL_MAX 12
#define KLE_KEY_MAX 256

namespace kle
{
    struct color_t
    {
        inline color_t()
            : m_r(0), m_g(0), m_b(0), m_a(0)
        {
        }

        bool operator==(const color_t &rhs) const { return m_r == rhs.m_r && m_g == rhs.m_g && m_b == rhs.m_b && m_a == rhs.m_a; }
        bool operator!=(const color_t &rhs) const { return !(*this == rhs); }

        unsigned char m_r;
        unsigned char m_g;
        unsigned char m_b;
        unsigned char m_a;
    };

    // strings are pointing into the raw json data and have
    // thus only a beginning and an end.
    struct str_t
    {
        inline str_t()
            : m_begin(-1), m_end(-1)
        {
        }
        inline str_t(int begin, int end) : m_begin(begin), m_end(end)
        {
        }

        bool is_null() const { return m_begin == -1 && m_begin == m_end; }

        color_t to_color(const char *json)
        {
            color_t c;
            return c;
        }

        int to_int(const char *json) const
        {
            if (m_begin == -1 || m_end == -1)
                return 0;
            const char *str = json + m_begin;
            const char *end = json + m_end;
            int val = 0;
            int sign = 1;
            if (*str == '-')
            {
                sign = -1;
                str++;
            }
            while (str < end)
            {
                val = val * 10 + (*str - '0');
                str++;
            }
            return val * sign;
        }

        int m_begin;
        int m_end;
    };

    struct key_t
    {
        color_t m_color;
        str_t m_labels[KLE_LABEL_MAX];
        color_t m_textColor[KLE_LABEL_MAX];
        int m_textSize[KLE_LABEL_MAX];
        color_t m_default_textColor;
        int m_default_textSize;
        int m_x;
        int m_y;
        int m_width;
        int m_height;
        int m_x2;
        int m_y2;
        int m_width2;
        int m_height2;
        int m_rotation_x;
        int m_rotation_y;
        int m_rotation_angle;
        int m_decal;
        int m_ghost;
        int m_stepped;
        int m_nub;
        str_t m_profile;
        str_t m_sm;
        str_t m_sb;
        str_t m_st;
    };

    static void init(key_t key)
    {
        key.m_color = color_t();
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            key.m_labels[i] = str_t();
            key.m_textColor[i] = color_t();
            key.m_textSize[i] = 0;
        }
        key.m_default_textColor = color_t();
        key.m_default_textSize = 0;
        key.m_x = 0;
        key.m_y = 0;
        key.m_width = 0;
        key.m_height = 0;
        key.m_x2 = 0;
        key.m_y2 = 0;
        key.m_width2 = 0;
        key.m_height2 = 0;
        key.m_rotation_x = 0;
        key.m_rotation_y = 0;
        key.m_rotation_angle = 0;
        key.m_decal = 0;
        key.m_ghost = 0;
        key.m_stepped = 0;
        key.m_nub = 0;
        key.m_profile = str_t();
        key.m_sm = str_t();
        key.m_sb = str_t();
        key.m_st = str_t();
    }

    static key_t copy(key_t const &key)
    {
        key_t result;
        result.m_color = key.m_color;
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            result.m_labels[i] = key.m_labels[i];
            result.m_textColor[i] = key.m_textColor[i];
            result.m_textSize[i] = key.m_textSize[i];
        }
        result.m_default_textColor = key.m_default_textColor;
        result.m_default_textSize = key.m_default_textSize;
        result.m_x = key.m_x;
        result.m_y = key.m_y;
        result.m_width = key.m_width;
        result.m_height = key.m_height;
        result.m_x2 = key.m_x2;
        result.m_y2 = key.m_y2;
        result.m_width2 = key.m_width2;
        result.m_height2 = key.m_height2;
        result.m_rotation_x = key.m_rotation_x;
        result.m_rotation_y = key.m_rotation_y;
        result.m_rotation_angle = key.m_rotation_angle;
        result.m_decal = key.m_decal;
        result.m_ghost = key.m_ghost;
        result.m_stepped = key.m_stepped;
        result.m_nub = key.m_nub;
        result.m_profile = key.m_profile;
        result.m_sm = key.m_sm;
        result.m_sb = key.m_sb;
        result.m_st = key.m_st;
        return result;
    }

    // export class KeyboardMetadata {
    //   author: string = "";
    //   backcolor: string = "#eeeeee";
    //   background: { name: string; style: string } | null = null;
    //   name: string = "";
    //   notes: string = "";
    //   radii: string = "";
    //   switchBrand: string = "";
    //   switchMount: string = "";
    //   switchType: string = "";
    // }
    struct bkgrnd_t
    {
        str_t m_name;
        str_t m_style;
    };

    static void init(bkgrnd_t bkgrnd)
    {
        bkgrnd.m_name = str_t();
        bkgrnd.m_style = str_t();
    }

    struct kb_metadata_t
    {
        str_t m_author;
        color_t m_backcolor;
        bkgrnd_t m_background;
        str_t m_name;
        str_t m_notes;
        str_t m_radii;
        str_t m_switchBrand;
        str_t m_switchMount;
        str_t m_switchType;
    };

    static void init(kb_metadata_t &kb_metadata)
    {
        kb_metadata.m_author = str_t();
        kb_metadata.m_backcolor = color_t();
        kb_metadata.m_background.m_name = str_t();
        kb_metadata.m_background.m_style = str_t();
        kb_metadata.m_name = str_t();
        kb_metadata.m_notes = str_t();
        kb_metadata.m_radii = str_t();
        kb_metadata.m_switchBrand = str_t();
        kb_metadata.m_switchMount = str_t();
        kb_metadata.m_switchType = str_t();
    }

    // export class Keyboard {
    //   meta: KeyboardMetadata = new KeyboardMetadata();
    //   keys: Key[] = [];
    // }
    struct kb_t
    {
        kb_metadata_t m_meta;
        int m_nb_keys;
        key_t m_keys[KLE_KEY_MAX];
    };

    static void init(kb_t &kb)
    {
        init(kb.m_meta);
        kb.m_nb_keys = 0;
        for (int i = 0; i < KLE_KEY_MAX; ++i)
        {
            init(kb.m_keys[i]);
        }
    }

    static void add(kb_t &kb, key_t const &key)
    {
        kb.m_keys[kb.m_nb_keys] = key;
        ++kb.m_nb_keys;
    }

    // export module Serial {
    //   // Helper to copy an object; doesn't handle loops/circular refs, etc.
    //   function copy(o: any): any {
    //     if (typeof o !== "object") {
    //       return o; // primitive value
    //     } else if (o instanceof Array) {
    //       var result: any[] = [];
    //       for (var i = 0; i < o.length; i++) {
    //         result[i] = copy(o[i]);
    //       }
    //       return result;
    //     } else {
    //       var oresult: object = Object.create(Object.getPrototypeOf(o));
    //       oresult.constructor();
    //       for (var prop in o) {
    //         oresult[prop] = copy(o[prop]);
    //       }
    //       return oresult;
    //     }
    //   }

    //   // Map from serialized label position to normalized position,
    //   // depending on the alignment flags.
    //   // prettier-ignore
    //   let labelMap: Array<Array<number>> = [
    //     //0  1  2  3  4  5  6  7  8  9 10 11   // align flags
    //     [ 0, 6, 2, 8, 9,11, 3, 5, 1, 4, 7,10], // 0 = no centering
    //     [ 1, 7,-1,-1, 9,11, 4,-1,-1,-1,-1,10], // 1 = center x
    //     [ 3,-1, 5,-1, 9,11,-1,-1, 4,-1,-1,10], // 2 = center y
    //     [ 4,-1,-1,-1, 9,11,-1,-1,-1,-1,-1,10], // 3 = center x & y
    //     [ 0, 6, 2, 8,10,-1, 3, 5, 1, 4, 7,-1], // 4 = center front (default)
    //     [ 1, 7,-1,-1,10,-1, 4,-1,-1,-1,-1,-1], // 5 = center front & x
    //     [ 3,-1, 5,-1,10,-1,-1,-1, 4,-1,-1,-1], // 6 = center front & y
    //     [ 4,-1,-1,-1,10,-1,-1,-1,-1,-1,-1,-1], // 7 = center front & x & y
    //   ];

    static int labelMap[8][12] = {
        {0, 6, 2, 8, 9, 11, 3, 5, 1, 4, 7, 10},
        {1, 7, -1, -1, 9, 11, 4, -1, -1, -1, -1, 10},
        {3, -1, 5, -1, 9, 11, -1, -1, 4, -1, -1, 10},
        {4, -1, -1, -1, 9, 11, -1, -1, -1, -1, -1, 10},
        {0, 6, 2, 8, 10, -1, 3, 5, 1, 4, 7, -1},
        {1, 7, -1, -1, 10, -1, 4, -1, -1, -1, -1, -1},
        {3, -1, 5, -1, 10, -1, -1, -1, 4, -1, -1, -1},
        {4, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1},
    };

    //   function reorderLabelsIn(labels, align) {
    //     var ret: Array<any> = [];
    //     for (var i = 0; i < labels.length; ++i) {
    //       if (labels[i]) ret[labelMap[align][i]] = labels[i];
    //     }
    //     return ret;
    //   }

    static void reorderLabelsIn(str_t labels[KLE_LABEL_MAX], jsmntok_t const &token, int align)
    {
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            if (labels[i].m_begin)
            {
                labels[i].m_begin = labels[labelMap[align][i]].m_begin;
                labels[i].m_end = labels[labelMap[align][i]].m_end;
            }
        }
    }

    struct item_t
    {
        str_t m_x;
        str_t m_y;
        str_t m_width;
        str_t m_height;
        str_t m_x2;
        str_t m_y2;
        str_t m_width2;
        str_t m_height2;
        str_t m_rotation_x;
        str_t m_rotation_y;
        str_t m_rotation_angle;
        str_t m_decal;
        str_t m_ghost;
        str_t m_stepped;
        str_t m_nub;
        str_t m_profile;
        str_t m_sm;
        str_t m_sb;
        str_t m_st;

        static void split(const char *json, item_t &item, const char *split, item_t *items_out, int max_items_out)
        {
            int nb_items = 0;
        };

        static bool key_is(jsmntok_t const &tk, const char *str)
        {
            return ((tk.end - tk.start) == strlen(str)) && memcmp(json + tk.start, str, strlen(str)) == 0;
        }

        static str_t value_to_str(const char *json, jsmntok_t const &tk)
        {
            return str_t(tk.start, tk.end);
        }

        static void parse(const char *json, jsmntok_t *tokens, int index, item_t &key)
        {
            // First token should be an object?
            int const parent = tokens[index].parent;
            while (parent == tokens[index].parent)
            {
                const jsmntok_t &token = tokens[index];
                if (token.type == JSMN_STRING)
                {
                    index++;

                    if (key_is(token, "x"))
                    {
                        key.m_x = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "y"))
                    {
                        key.m_y = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "width"))
                    {
                        key.m_width = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "height"))
                    {
                        key.m_height = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "x2"))
                    {
                        key.m_x2 = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "y2"))
                    {
                        key.m_y2 = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "width2"))
                    {
                        key.m_width2 = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "height2"))
                    {
                        key.m_height2 = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "rotation_x"))
                    {
                        key.m_rotation_x = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "rotation_y"))
                    {
                        key.m_rotation_y = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "rotation_angle"))
                    {
                        key.m_rotation_angle = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "decal"))
                    {
                        key.m_decal = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "ghost"))
                    {
                        key.m_ghost = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "stepped"))
                    {
                        key.m_stepped = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "nub"))
                    {
                        key.m_nub = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "profile"))
                    {
                        key.m_profile = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "sm"))
                    {
                        key.m_sm = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "sb"))
                    {
                        key.m_sb = value_to_str(tokens[index]);
                    }
                    else if (key_is(token, "st"))
                    {
                        key.m_st = value_to_str(tokens[index]);
                    }
                    index++;
                }
            }

            //   function deserializeError(msg, data?) {
            //     throw "Error: " + msg + (data ? ":\n  " + JSON5.stringify(data) : "");
            //   }
            //

            //   export function deserialize(rows: Array<any>): Keyboard {
            //     if (!(rows instanceof Array))
            //       deserializeError("expected an array of objects");
            //
            //     // Initialize with defaults
            //     let current: Key = new Key();
            //     let kbd = new Keyboard();
            //     var align = 4;
            //
            //     for (var r = 0; r < rows.length; ++r) {
            //       if (rows[r] instanceof Array) {
            //         for (var k = 0; k < rows[r].length; ++k) {
            //           var item = rows[r][k];
            //           if (typeof item === "string") {
            //             var newKey: Key = copy(current);
            //
            //             // Calculate some generated values
            //             newKey.width2 =
            //               newKey.width2 === 0 ? current.width : current.width2;
            //             newKey.height2 =
            //               newKey.height2 === 0 ? current.height : current.height2;
            //             newKey.labels = reorderLabelsIn(item.split("\n"), align);
            //             newKey.textSize = reorderLabelsIn(newKey.textSize, align);
            //
            //             // Clean up the data
            //             for (var i = 0; i < 12; ++i) {
            //               if (!newKey.labels[i]) {
            //                 delete newKey.textSize[i];
            //                 delete newKey.textColor[i];
            //               }
            //               if (newKey.textSize[i] == newKey.default.textSize)
            //                 delete newKey.textSize[i];
            //               if (newKey.textColor[i] == newKey.default.textColor)
            //                 delete newKey.textColor[i];
            //             }
            //
            //             // Add the key!
            //             kbd.keys.push(newKey);
            //
            //             // Set up for the next key
            //             current.x += current.width;
            //             current.width = current.height = 1;
            //             current.x2 = current.y2 = current.width2 = current.height2 = 0;
            //             current.nub = current.stepped = current.decal = false;
            //           } else {
            //             if (
            //               k != 0 &&
            //               (item.r != null || item.rx != null || item.ry != null)
            //             ) {
            //               deserializeError(
            //                 "rotation can only be specified on the first key in a row",
            //                 item
            //               );
            //             }
            //             if (item.r != null) current.rotation_angle = item.r;
            //             if (item.rx != null) current.rotation_x = item.rx;
            //             if (item.ry != null) current.rotation_y = item.ry;
            //             if (item.a != null) align = item.a;
            //             if (item.f) {
            //               current.default.textSize = item.f;
            //               current.textSize = [];
            //             }
            //             if (item.f2)
            //               for (var i = 1; i < 12; ++i) current.textSize[i] = item.f2;
            //             if (item.fa) current.textSize = item.fa;
            //             if (item.p) current.profile = item.p;
            //             if (item.c) current.color = item.c;
            //             if (item.t) {
            //               var split = item.t.split("\n");
            //               if (split[0] != "") current.default.textColor = split[0];
            //               current.textColor = reorderLabelsIn(split, align);
            //             }
            //             if (item.x) current.x += item.x;
            //             if (item.y) current.y += item.y;
            //             if (item.w) current.width = current.width2 = item.w;
            //             if (item.h) current.height = current.height2 = item.h;
            //             if (item.x2) current.x2 = item.x2;
            //             if (item.y2) current.y2 = item.y2;
            //             if (item.w2) current.width2 = item.w2;
            //             if (item.h2) current.height2 = item.h2;
            //             if (item.n) current.nub = item.n;
            //             if (item.l) current.stepped = item.l;
            //             if (item.d) current.decal = item.d;
            //             if (item.g != null) current.ghost = item.g;
            //             if (item.sm) current.sm = item.sm;
            //             if (item.sb) current.sb = item.sb;
            //             if (item.st) current.st = item.st;
            //           }
            //         }
            //
            //         // End of the row
            //         current.y++;
            //         current.x = current.rotation_x;
            //       } else if (typeof rows[r] === "object") {
            //         if (r != 0) {
            //           deserializeError(
            //             "keyboard metadata must the be first element",
            //             rows[r]
            //           );
            //         }
            //         for (let prop in kbd.meta) {
            //           if (rows[r][prop]) kbd.meta[prop] = rows[r][prop];
            //         }
            //       } else {
            //         deserializeError("unexpected", rows[r]);
            //       }
            //     }
            //     return kbd;
            //   }

            bool read_kb_layout(const char *filename, kb_t &kb)
            {
                // read raw file content
                FILE *pFile;
                pFile = fopen(filename, "rb");
                if (pFile == NULL)
                {
                    printf("Error opening file %s\n", filename);
                    return false;
                }
                fseek(pFile, 0, SEEK_END);
                long lSize = ftell(pFile);
                rewind(pFile);
                char *buffer = (char *)malloc(sizeof(char) * lSize);
                if (buffer == NULL)
                {
                    printf("Error allocating memory\n");
                    return false;
                }
                fread(buffer, 1, lSize, pFile);
                fclose(pFile);

                // parse json
                jsmn_parser parser;
                jsmntok_t *tokens = (jsmntok_t *)malloc(sizeof(jsmntok_t) * 1024);
                jsmn_init(&parser, tokens, 1024);
                int num_tokens = jsmn_parse(&parser, buffer, strlen(buffer));
                if (num_tokens < 0)
                {
                    printf("Failed to parse JSON: %d\n", num_tokens);
                    return false;
                }

                // Assume the top-level element is an object
                if (tokens[0].type != JSMN_OBJECT)
                {
                    printf("Object expected\n");
                    return 1;
                }

                // Initialize with defaults
                key_t current;
                init(current);
                init(kb);

                int align = 4;

                for (int r = 1; r < num_tokens; ++r)
                {
                    jsmntok_t &item = tokens[r];
                    if (item.type == JSMN_STRING)
                    {
                        key_t newKey = copy(current);

                        newKey.m_width2 = newKey.m_width2 == 0 ? current.m_width : current.m_width2;
                        newKey.m_height2 = newKey.m_height2 == 0 ? current.m_height : current.m_height2;
                        reorderLabelsIn(newKey.m_labels, item, align);

                        for (int i = 0; i < 12; ++i)
                        {
                            if (newKey.m_labels[i].is_null())
                            {
                                newKey.m_textSize[i] = 0;
                                newKey.m_textColor[i] = color_t();
                            }
                            if (newKey.m_textSize[i] == newKey.m_default_textSize)
                                newKey.m_textSize[i] = 0;
                            if (newKey.m_textColor[i] == newKey.m_default_textColor)
                                newKey.m_textColor[i] = color_t();
                        }

                        // Add the key!
                        add(kb, newKey);

                        // Set up for the next key
                        current.m_x += current.m_width;
                        current.m_width = current.m_height = 1;
                        current.m_x2 = current.m_y2 = current.m_width2 = current.m_height2 = 0;
                        current.m_nub = current.m_stepped = current.m_decal = false;
                    }
                    else if (item.type == JSMN_OBJECT)
                    {
                        item_t newItem;
                        parse(parser->m_begin, tokens, r, newItem);

                        if (k != 0 && (!newItem.m_r.is_null() || !newItem.m_rx.is_null() || !newItem.m_ry.is_null()))
                        {
                            // "rotation can only be specified on the first key in a row"
                            return false;
                        }

                        if (!item.r.is_null())
                            current.m_rotation = item.r.to_int();
                        if (!item.rx.is_null())
                            current.m_rotation_x = item.rx.to_int();
                        if (!item.ry.is_null())
                            current.m_rotation_y = item.ry.to_int();

                        if (!item.a.is_null())
                            align = item.a.to_int();

                        if (!item.f.is_null())
                        {
                            current.m_default_textSize = item.f.to_int();
                            for (int i = 0; i < 12; ++i)
                                current.m_textSize[i] = current.m_default_textSize;
                        }
                        if (!item.f2.is_null())
                        {
                            for (int i = 0; i < 12; ++i)
                                current.textSize[i] = item.f2;
                        }
                        if (!item.fa.is_null())
                            current.textSize = item.fa.to_int();

                        if (!item.p.is_null())
                            current.m_profile = item.p.to_int();
                        if (!item.c.is_null())
                            current.m_color = item.c.to_color(parser->m_begin);
                        if (!item.t.is_null())
                        {
                            str_t colors[12];
                            item_t::split(parser->m_begin, item.t, "\n", colors, 12);
                            if (!colors[0].is_null())
                                current.m_default_textColor = colors[0].to_color(parser->m_begin);
                            current.m_textColor = reorderLabelsIn(colors, align);
                        }
                        if (!item.x.is_null())
                            current.m_x += item.x.to_int();
                        if (!item.y.is_null())
                            current.m_y += item.y.to_int();
                        if (!item.w.is_null())
                        {
                            current.m_width = current.m_width2 = item.w.to_int();
                        }
                        if (!item.h.is_null())
                        {
                            current.m_height = current.m_height2 = item.h.to_int();
                        }
                        if (!item.x2.is_null())
                            current.m_x2 = item.x2.to_int();
                        if (!item.y2.is_null())
                            current.m_y2 = item.y2.to_int();
                        if (!item.w2.is_null())
                            current.m_width2 = item.w2.to_int();
                        if (!item.h2.is_null())
                            current.m_height2 = item.h2.to_int();
                        if (!item.n.is_null())
                            current.m_nub = item.n.to_int();
                        if (!item.l.is_null())
                            current.m_stepped = item.l.to_int();
                        if (!item.d.is_null())
                            current.m_decal = item.d.to_int();
                        if (!item.g.is_null())
                            current.m_ghost = item.g.to_int();
                        if (!item.sm.is_null())
                            current.m_sm = item.sm.to_int();
                        if (!item.sb.is_null())
                            current.m_sb = item.sb.to_int();
                        if (!item.st.is_null())
                            current.m_st = item.st.to_int();

                        current.y++;
                        current.x = current.rotation_x;
                    } else if (item.type == JSMN_ARRAY) {
            //       } else if (typeof rows[r] === "object") {
            //         if (r != 0) {
            //           deserializeError(
            //             "keyboard metadata must the be first element",
            //             rows[r]
            //           );
            //         }
            //         for (let prop in kbd.meta) {
            //           if (rows[r][prop]) kbd.meta[prop] = rows[r][prop];
            //         }
            //       } else {
            //         deserializeError("unexpected", rows[r]);
            //       }

            //       r++;
                }
            }

        } // namespace kle
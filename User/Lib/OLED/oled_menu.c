/**
 * @file oled_menu.c
 * @brief OLED Menu System Implementation
 * @version 1.0
 * @date 2025-12-21
 */

#include "oled_menu.h"

/* ========== Menu Initialization ========== */

void OLED_MenuInit(Menu* menu, MenuItem* items, uint8_t item_count) {
    menu->items = items;
    menu->item_count = item_count;
    menu->selected = 0;
    menu->scroll_offset = 0;
    menu->style = MENU_STYLE_LIST;
    menu->title = NULL;
    menu->parent = NULL;
    
    // Set parent reference for submenus
    for(uint8_t i = 0; i < item_count; i++) {
        if(items[i].type == MENU_ITEM_SUBMENU && items[i].submenu != NULL) {
            items[i].submenu->parent = menu;
        }
    }
}

void OLED_MenuSetTitle(Menu* menu, const char* title) {
    menu->title = title;
}

void OLED_MenuSetStyle(Menu* menu, MenuStyle style) {
    menu->style = style;
}

/* ========== Menu Navigation ========== */

void OLED_MenuNext(Menu* menu) {
    if(menu->selected < menu->item_count - 1) {
        menu->selected++;
        
        // Auto-scroll
        if(menu->selected >= menu->scroll_offset + MENU_MAX_VISIBLE) {
            menu->scroll_offset = menu->selected - MENU_MAX_VISIBLE + 1;
        }
    }
}

void OLED_MenuPrev(Menu* menu) {
    if(menu->selected > 0) {
        menu->selected--;
        
        // Auto-scroll
        if(menu->selected < menu->scroll_offset) {
            menu->scroll_offset = menu->selected;
        }
    }
}

Menu* OLED_MenuSelect(Menu* menu) {
    MenuItem* item = &menu->items[menu->selected];
    
    switch(item->type) {
        case MENU_ITEM_ACTION:
            if(item->callback != NULL) {
                item->callback();
            }
            return NULL;
            
        case MENU_ITEM_SUBMENU:
            return item->submenu;
            
        case MENU_ITEM_BACK:
            return menu->parent;
            
        default:
            return NULL;
    }
}

Menu* OLED_MenuBack(Menu* menu) {
    return menu->parent;
}

/* ========== Value Editing ========== */

void OLED_MenuValueInc(Menu* menu) {
    MenuItem* item = &menu->items[menu->selected];
    
    if(item->value == NULL) return;
    
    switch(item->type) {
        case MENU_ITEM_VALUE_INT:
            if(item->value->value < item->value->max) {
                item->value->value += item->value->step;
                if(item->value->value > item->value->max) {
                    item->value->value = item->value->max;
                }
            }
            break;
            
        case MENU_ITEM_VALUE_LIST:
            if(item->value->value < item->value->option_count - 1) {
                item->value->value++;
            }
            break;
            
        default:
            break;
    }
}

void OLED_MenuValueDec(Menu* menu) {
    MenuItem* item = &menu->items[menu->selected];
    
    if(item->value == NULL) return;
    
    switch(item->type) {
        case MENU_ITEM_VALUE_INT:
            if(item->value->value > item->value->min) {
                item->value->value -= item->value->step;
                if(item->value->value < item->value->min) {
                    item->value->value = item->value->min;
                }
            }
            break;
            
        case MENU_ITEM_VALUE_LIST:
            if(item->value->value > 0) {
                item->value->value--;
            }
            break;
            
        default:
            break;
    }
}

void OLED_MenuValueToggle(Menu* menu) {
    MenuItem* item = &menu->items[menu->selected];
    
    if(item->type == MENU_ITEM_VALUE_BOOL && item->value != NULL) {
        item->value->value = !item->value->value;
    }
}

/* ========== Menu Drawing ========== */

void OLED_MenuDraw(OLED_Handle* oled, Menu* menu) {
    switch(menu->style) {
        case MENU_STYLE_ICON:
            OLED_MenuDrawIcon(oled, menu);
            break;
        case MENU_STYLE_FULL:
            OLED_MenuDrawFull(oled, menu);
            break;
        case MENU_STYLE_LIST:
        default:
            OLED_MenuDrawList(oled, menu);
            break;
    }
}

void OLED_MenuDrawList(OLED_Handle* oled, Menu* menu) {
    uint8_t y = 0;
    
    // Draw title if present
    if(menu->title != NULL) {
        OLED_DrawStringAlign(oled, oled->width / 2, 0, menu->title, OLED_COLOR_WHITE, OLED_ALIGN_CENTER);
        OLED_DrawHLine(oled, 0, 10, oled->width, OLED_COLOR_WHITE);
        y = 12;
    }
    
    // Draw menu items
    uint8_t visible_count = (menu->title != NULL) ? 3 : MENU_MAX_VISIBLE;
    
    for(uint8_t i = 0; i < visible_count && (i + menu->scroll_offset) < menu->item_count; i++) {
        uint8_t item_index = i + menu->scroll_offset;
        MenuItem* item = &menu->items[item_index];
        uint8_t item_y = y + i * MENU_ITEM_HEIGHT;
        
        // Draw selection indicator
        if(item_index == menu->selected) {
            OLED_FillRect(oled, 0, item_y, oled->width, MENU_ITEM_HEIGHT, OLED_COLOR_WHITE);
            OLED_DrawString(oled, 4, item_y + 2, item->text, OLED_COLOR_BLACK);
        } else {
            OLED_DrawString(oled, 4, item_y + 2, item->text, OLED_COLOR_WHITE);
        }
        
        // Draw value if applicable
        if(item->value != NULL) {
            char value_str[16];
            
            switch(item->type) {
                case MENU_ITEM_VALUE_INT:
                    snprintf(value_str, sizeof(value_str), "%ld", (long)item->value->value);
                    break;
                    
                case MENU_ITEM_VALUE_BOOL:
                    snprintf(value_str, sizeof(value_str), "%s", item->value->value ? "ON" : "OFF");
                    break;
                    
                case MENU_ITEM_VALUE_LIST:
                    if(item->value->options != NULL && item->value->value < item->value->option_count) {
                        snprintf(value_str, sizeof(value_str), "%s", item->value->options[item->value->value]);
                    }
                    break;
                    
                default:
                    value_str[0] = '\0';
                    break;
            }
            
            if(value_str[0] != '\0') {
                OLED_Color color = (item_index == menu->selected) ? OLED_COLOR_BLACK : OLED_COLOR_WHITE;
                OLED_DrawStringAlign(oled, oled->width - 4, item_y + 2, value_str, color, OLED_ALIGN_RIGHT);
            }
        }
        
        // Draw submenu indicator
        if(item->type == MENU_ITEM_SUBMENU) {
            OLED_Color color = (item_index == menu->selected) ? OLED_COLOR_BLACK : OLED_COLOR_WHITE;
            OLED_DrawString(oled, oled->width - 10, item_y + 2, ">", color);
        }
    }
    
    // Draw scroll indicators
    if(menu->scroll_offset > 0) {
        OLED_DrawString(oled, oled->width / 2, y, "^", OLED_COLOR_WHITE);
    }
    if(menu->scroll_offset + visible_count < menu->item_count) {
        OLED_DrawString(oled, oled->width / 2, oled->height - 8, "v", OLED_COLOR_WHITE);
    }
}

void OLED_MenuDrawIcon(OLED_Handle* oled, Menu* menu) {
    // Icon menu - simplified implementation
    uint8_t icon_size = 32;
    uint8_t spacing = 8;
    uint8_t start_x = (oled->width - icon_size) / 2;
    uint8_t start_y = 16;
    
    MenuItem* item = &menu->items[menu->selected];
    
    // Draw icon if present
    if(item->icon != NULL) {
        OLED_DrawBitmap(oled, start_x, start_y, item->icon, OLED_COLOR_WHITE);
    } else {
        OLED_DrawRect(oled, start_x, start_y, icon_size, icon_size, OLED_COLOR_WHITE);
    }
    
    // Draw text
    OLED_DrawStringAlign(oled, oled->width / 2, start_y + icon_size + spacing, 
                         item->text, OLED_COLOR_WHITE, OLED_ALIGN_CENTER);
    
    // Draw navigation indicators
    if(menu->selected > 0) {
        OLED_DrawString(oled, 0, oled->height / 2, "<", OLED_COLOR_WHITE);
    }
    if(menu->selected < menu->item_count - 1) {
        OLED_DrawString(oled, oled->width - 8, oled->height / 2, ">", OLED_COLOR_WHITE);
    }
}

void OLED_MenuDrawFull(OLED_Handle* oled, Menu* menu) {
    MenuItem* item = &menu->items[menu->selected];
    
    // Draw title
    OLED_DrawStringAlign(oled, oled->width / 2, 0, item->text, OLED_COLOR_WHITE, OLED_ALIGN_CENTER);
    OLED_DrawHLine(oled, 0, 10, oled->width, OLED_COLOR_WHITE);
    
    // Draw value/content in center
    if(item->value != NULL) {
        char value_str[32];
        
        switch(item->type) {
            case MENU_ITEM_VALUE_INT:
                snprintf(value_str, sizeof(value_str), "%ld", (long)item->value->value);
                break;
                
            case MENU_ITEM_VALUE_BOOL:
                snprintf(value_str, sizeof(value_str), "%s", item->value->value ? "ON" : "OFF");
                break;
                
            case MENU_ITEM_VALUE_LIST:
                if(item->value->options != NULL && item->value->value < item->value->option_count) {
                    snprintf(value_str, sizeof(value_str), "%s", item->value->options[item->value->value]);
                }
                break;
                
            default:
                value_str[0] = '\0';
                break;
        }
        
        OLED_DrawStringAlign(oled, oled->width / 2, oled->height / 2 - 4, 
                             value_str, OLED_COLOR_WHITE, OLED_ALIGN_CENTER);
    }
}

/* ========== Helper Functions ========== */

MenuItem OLED_MenuCreateAction(const char* text, void (*callback)(void)) {
    MenuItem item = {
        .text = text,
        .callback = callback,
        .type = MENU_ITEM_ACTION,
        .icon = NULL,
        .value = NULL,
        .submenu = NULL
    };
    return item;
}

MenuItem OLED_MenuCreateSubmenu(const char* text, Menu* submenu) {
    MenuItem item = {
        .text = text,
        .callback = NULL,
        .type = MENU_ITEM_SUBMENU,
        .icon = NULL,
        .value = NULL,
        .submenu = submenu
    };
    return item;
}

MenuItem OLED_MenuCreateValueInt(const char* text, MenuValue* value) {
    MenuItem item = {
        .text = text,
        .callback = NULL,
        .type = MENU_ITEM_VALUE_INT,
        .icon = NULL,
        .value = value,
        .submenu = NULL
    };
    return item;
}

MenuItem OLED_MenuCreateValueBool(const char* text, MenuValue* value) {
    MenuItem item = {
        .text = text,
        .callback = NULL,
        .type = MENU_ITEM_VALUE_BOOL,
        .icon = NULL,
        .value = value,
        .submenu = NULL
    };
    return item;
}

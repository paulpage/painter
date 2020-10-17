#ifndef COMMON_H
#define COMMON_H

enum Tool {
    TOOL_PENCIL,
    TOOL_PAINTBRUSH,
    TOOL_COLOR_PICKER,
    TOOL_PAINT_BUCKET,
    TOOL_SPRAY_CAN,
    TOOL_ERASER,
    TOOL_MOVE,
    TOOL_RECTANGLE_SELECT,
    TOOL_LINE,

    FINAL_TOOL_COUNT, // Marks the end of the enum
};

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif // COMMON_H

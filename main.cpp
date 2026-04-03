// Updated main.cpp for geometric visualization
#include <iostream>
#include <graphics.h> // Assuming graphics.h is the graphics library used

void drawVisualization() {
    // Draw semicircle arc
    arc(200, 200, 0, 180, 100); // Example coordinates and radius

    // Draw angle lines
    line(200, 200, 100, 150);
    line(200, 200, 100, 250);

    // Draw colored squares
    setfillstyle(SOLID_FILL, RED);
    floodfill(50, 50, WHITE); // Example square position
    setfillstyle(SOLID_FILL, BLUE);
    floodfill(150, 50, WHITE);
}

int main() {
    initwindow(400, 400);
    drawVisualization();
    getch();
    closegraph();
    return 0;
}
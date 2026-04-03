#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

const int CANVAS_WIDTH = 1200;
const int CANVAS_HEIGHT = 800;
const int DIAMETER = 300;
const int CENTER_X = CANVAS_WIDTH / 2;
const int BOTTOM_Y = CANVAS_HEIGHT - 400;
const int LEFT_X = (CANVAS_WIDTH - DIAMETER) / 2;
const int RIGHT_X = LEFT_X + DIAMETER;

// Generate Gaussian noise using Box-Muller transform
sf::Image generateGaussianNoiseImage(unsigned int width, unsigned int height) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> norm(0.0, 1.0);

    sf::Image img;
    img.create(width, height, sf::Color::Black);

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            auto gauss = [&]() -> sf::Uint8 {
                double val = norm(gen) * 127.5 + 127.5;
                val = std::clamp(val, 0.0, 255.0);
                return static_cast<sf::Uint8>(val);
            };
            sf::Color pixel(gauss(), gauss(), gauss());
            img.setPixel(x, y, pixel);
        }
    }
    return img;
}

// Mix images: result = sqrt(1-β)*original + sqrt(β)*noise
sf::Image mixImages(const sf::Image& original, const sf::Image& noise,
                    double sqrtBetaVal, double sqrtOneMinusBetaVal) {
    sf::Image result = original;
    unsigned int w = original.getSize().x;
    unsigned int h = original.getSize().y;

    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            sf::Color orig = original.getPixel(x, y);
            sf::Color noi = noise.getPixel(x, y);

            int r = static_cast<int>(sqrtOneMinusBetaVal * orig.r + sqrtBetaVal * noi.r);
            int g = static_cast<int>(sqrtOneMinusBetaVal * orig.g + sqrtBetaVal * noi.g);
            int b = static_cast<int>(sqrtOneMinusBetaVal * orig.b + sqrtBetaVal * noi.b);

            result.setPixel(x, y, sf::Color(
                static_cast<sf::Uint8>(std::clamp(r, 0, 255)),
                static_cast<sf::Uint8>(std::clamp(g, 0, 255)),
                static_cast<sf::Uint8>(std::clamp(b, 0, 255))
            ));
        }
    }
    return result;
}

// Draw a circle outline
void drawCircleOutline(sf::RenderWindow& window, float cx, float cy, float radius, const sf::Color& color) {
    const int segments = 100;
    sf::VertexArray line(sf::LineStrip, segments);
    for (int i = 0; i < segments; ++i) {
        float angle = M_PI * (float)i / (segments - 1);
        line[i].position = sf::Vector2f(cx + radius * std::cos(angle), cy - radius * std::sin(angle));
        line[i].color = color;
    }
    window.draw(line);
}

// Format number as string with precision
std::string formatNumber(double value, int precision = 8) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(CANVAS_WIDTH, CANVAS_HEIGHT), "Diffusion Model Visualization");
    window.setFramerateLimit(60);

    // Load or generate a test image
    sf::Image originalImage;
    if (!originalImage.loadFromFile("sample.png")) {
        // Create a simple gradient test image
        originalImage.create(256, 256, sf::Color::White);
        for (unsigned int y = 0; y < 256; ++y) {
            for (unsigned int x = 0; x < 256; ++x) {
                sf::Uint8 val = static_cast<sf::Uint8>((x + y) / 2);
                originalImage.setPixel(x, y, sf::Color(val, val, val));
            }
        }
    }

    // Generate noise image
    sf::Image noiseImage = generateGaussianNoiseImage(originalImage.getSize().x, originalImage.getSize().y);

    // Create textures
    sf::Texture originalTex, noiseTex, mixedTex;
    originalTex.loadFromImage(originalImage);
    noiseTex.loadFromImage(noiseImage);

    // Load or create font
    sf::Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    double theta = M_PI / 3.0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseMoved) {
                int mouseX = event.mouseMove.x;
                int mouseY = event.mouseMove.y;

                // Calculate theta based on mouse position
                float dx = mouseX - CENTER_X;
                float dy = BOTTOM_Y - mouseY;
                theta = std::atan2(dy, dx);
                theta = std::clamp(theta, 0.0, M_PI);
            }
        }

        double sqrtBeta = std::sin(0.5 * theta);
        double sqrtOneMinusBeta = std::cos(0.5 * theta);
        double beta = sqrtBeta * sqrtBeta;
        double oneMinusBeta = sqrtOneMinusBeta * sqrtOneMinusBeta;

        // Mix images
        sf::Image mixedImage = mixImages(originalImage, noiseImage, sqrtBeta, sqrtOneMinusBeta);
        mixedTex.loadFromImage(mixedImage);

        window.clear(sf::Color::White);

        // Draw semicircle arc
        drawCircleOutline(window, CENTER_X, BOTTOM_Y, DIAMETER / 2.0f, sf::Color(100, 100, 100));

        // Draw baseline
        sf::Vertex baseline[] = {
            sf::Vertex(sf::Vector2f(LEFT_X, BOTTOM_Y), sf::Color(100, 100, 100)),
            sf::Vertex(sf::Vector2f(RIGHT_X, BOTTOM_Y), sf::Color(100, 100, 100))
        };
        window.draw(baseline, 2, sf::Lines);

        // Calculate point on arc
        float x = CENTER_X + (DIAMETER / 2.0f) * std::cos(theta);
        float y = BOTTOM_Y - (DIAMETER / 2.0f) * std::sin(theta);

        // Draw angle lines (left - cyan, right - magenta)
        sf::Vertex cyanLine[] = {
            sf::Vertex(sf::Vector2f(LEFT_X, BOTTOM_Y), sf::Color(0, 204, 204)),
            sf::Vertex(sf::Vector2f(x, y), sf::Color(0, 204, 204))
        };
        window.draw(cyanLine, 2, sf::Lines);

        sf::Vertex magentaLine[] = {
            sf::Vertex(sf::Vector2f(x, y), sf::Color(204, 0, 204)),
            sf::Vertex(sf::Vector2f(RIGHT_X, BOTTOM_Y), sf::Color(204, 0, 204))
        };
        window.draw(magentaLine, 2, sf::Lines);

        // Draw colored squares
        sf::RectangleShape cyanSquare(sf::Vector2f(sqrtBeta * DIAMETER, sqrtBeta * DIAMETER));
        cyanSquare.setPosition(x, y);
        cyanSquare.setFillColor(sf::Color(0, 200, 200, 50));
        window.draw(cyanSquare);

        sf::RectangleShape magentaSquare(sf::Vector2f(sqrtOneMinusBeta * DIAMETER, sqrtOneMinusBeta * DIAMETER));
        magentaSquare.setPosition(x, y);
        magentaSquare.setFillColor(sf::Color(200, 0, 200, 50));
        window.draw(magentaSquare);

        // Draw parameters text
        sf::Text paramText;
        paramText.setFont(font);
        paramText.setCharacterSize(20);
        paramText.setFillColor(sf::Color::Black);
        std::string paramStr = "β=" + formatNumber(beta) + "; 1-β=" + formatNumber(oneMinusBeta);
        paramText.setString(paramStr);
        paramText.setPosition(CENTER_X - 300, 30);
        window.draw(paramText);

        // Draw images
        sf::Sprite noiseSprite(noiseTex);
        noiseSprite.setPosition(20, CANVAS_HEIGHT - noiseImage.getSize().y - 20);
        window.draw(noiseSprite);

        sf::Text noiseLabelText;
        noiseLabelText.setFont(font);
        noiseLabelText.setCharacterSize(16);
        noiseLabelText.setFillColor(sf::Color::Black);
        noiseLabelText.setString("sqrt(β)=" + formatNumber(sqrtBeta, 6));
        noiseLabelText.setPosition(20, CANVAS_HEIGHT - noiseImage.getSize().y - 60);
        window.draw(noiseLabelText);

        sf::Sprite mixedSprite(mixedTex);
        mixedSprite.setPosition(CENTER_X - noiseImage.getSize().x / 2, CANVAS_HEIGHT - noiseImage.getSize().y - 20);
        window.draw(mixedSprite);

        sf::Sprite originalSprite(originalTex);
        originalSprite.setPosition(CANVAS_WIDTH - originalImage.getSize().x - 20, CANVAS_HEIGHT - originalImage.getSize().y - 20);
        window.draw(originalSprite);

        sf::Text originalLabelText;
        originalLabelText.setFont(font);
        originalLabelText.setCharacterSize(16);
        originalLabelText.setFillColor(sf::Color::Black);
        originalLabelText.setString("sqrt(1-β)=" + formatNumber(sqrtOneMinusBeta, 6));
        originalLabelText.setPosition(CANVAS_WIDTH - originalImage.getSize().x / 2 - 100, CANVAS_HEIGHT - originalImage.getSize().y - 60);
        window.draw(originalLabelText);

        window.display();
    }

    return 0;
}
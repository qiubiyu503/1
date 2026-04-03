#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Function to load or generate an image
sf::Image loadImage(const std::string &path) {
    sf::Image image;
    if (!image.loadFromFile(path)) {
        std::cerr << "Error loading image" << std::endl;
    }
    return image;
}

// Function to generate Gaussian noise
std::vector<sf::Color> generateGaussianNoise(int width, int height, float mean, float stddev) {
    std::vector<sf::Color> noise;
    std::srand(std::time(0));
    for (int i = 0; i < width * height; ++i) {
        float r = mean + stddev * ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 2;
        float g = mean + stddev * ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 2;
        float b = mean + stddev * ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 2;
        noise.emplace_back(static_cast<unsigned char>(std::clamp(static_cast<int>(r), 0, 255)), 
                           static_cast<unsigned char>(std::clamp(static_cast<int>(g), 0, 255)), 
                           static_cast<unsigned char>(std::clamp(static_cast<int>(b), 0, 255)), 255);
    }
    return noise;
}

// Function to mix images based on mouse position
sf::Image mixImages(const sf::Image &baseImage, const std::vector<sf::Color> &noise, float factor) {
    sf::Image mixedImage = baseImage;
    for (size_t i = 0; i < baseImage.getSize().x * baseImage.getSize().y; ++i) {
        int x = i % baseImage.getSize().x;
        int y = i / baseImage.getSize().x;
        sf::Color baseColor = baseImage.getPixel(x, y);
        sf::Color noiseColor = noise[i];
        mixedImage.setPixel(x, y, sf::Color(
            static_cast<sf::Uint8>(std::clamp(static_cast<int>(baseColor.r * (1 - factor) + noiseColor.r * factor), 0, 255)),
            static_cast<sf::Uint8>(std::clamp(static_cast<int>(baseColor.g * (1 - factor) + noiseColor.g * factor), 0, 255)),
            static_cast<sf::Uint8>(std::clamp(static_cast<int>(baseColor.b * (1 - factor) + noiseColor.b * factor), 0, 255)),
            255
        ));
    }
    return mixedImage;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Diffusion Model Visualization");
    sf::Image baseImage = loadImage("path/to/image.png"); // Update with a valid image path
    sf::Texture baseTexture;
    baseTexture.loadFromImage(baseImage);
    sf::Sprite sprite(baseTexture);

    // Generate Gaussian noise
    auto noise = generateGaussianNoise(baseImage.getSize().x, baseImage.getSize().y, 128, 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Get mouse position and calculate factor
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float factor = std::sqrt(static_cast<float>(mousePos.x) / window.getSize().x);
        factor = std::clamp(factor, 0.0f, 1.0f);

        // Mix images based on mouse position
        sf::Image mixedImage = mixImages(baseImage, noise, factor);
        baseTexture.loadFromImage(mixedImage);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
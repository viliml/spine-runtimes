/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated January 1, 2020. Replaces all prior versions.
 *
 * Copyright (c) 2013-2020, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software
 * or otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THE SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <iostream>
#include <spine/spine-sfml.h>
#include <spine/Debug.h>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace spine;
#include <memory>

shared_ptr<SkeletonData> readSkeletonJsonData (const String& filename, Atlas* atlas, float scale) {
	SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filename);
	if (!skeletonData) {
		printf("%s\n", json.getError().buffer());
		exit(0);
	}
	return shared_ptr<SkeletonData>(skeletonData);
}

shared_ptr<SkeletonData> readSkeletonBinaryData (const char* filename, Atlas* atlas, float scale) {
	SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonDataFile(filename);
	if (!skeletonData) {
		printf("%s\n", binary.getError().buffer());
		exit(0);
	}
	return shared_ptr<SkeletonData>(skeletonData);
}

void hoshino(SkeletonData* skeletonData, Atlas* atlas, float scale) {
	SP_UNUSED(atlas);
	float w = skeletonData->getWidth() * scale;
	float h = skeletonData->getHeight() * scale;
	float x = skeletonData->getX() * scale;
	float y = skeletonData->getY() * scale;
	float a = floor(-w - x), b = floor(-h - y), c = ceil(-x), d = ceil(-y);
	printf("%f %f %f %f", w, h, x, y);

	SkeletonDrawable drawable(skeletonData);
	drawable.timeScale = 1;
	drawable.setUsePremultipliedAlpha(false);

	Skeleton* skeleton = drawable.skeleton;
	//skeleton->setPosition(w + x, y + h);
	{
		auto vec = skeletonData->getAnimations();
		for (int i = 0; i < vec.size(); ++i) {
			auto* anim = vec[i];
			printf("%s\n", anim->getName().buffer());

			drawable.state->setAnimation(0, anim->getName(), true);
			skeleton->updateWorldTransform();
			drawable.update(0.0f);

			sf::RenderTexture texture;
			texture.create(512, 480);
			texture.clear(sf::Color::Transparent);
			//sf::View view = texture.getDefaultView();
			//view.setViewport({ 0.262, 0.059, 0.5, 0.227 });
			texture.setView(sf::View{ {a + 352, b + 64, 512, 480} });
			texture.draw(drawable);
			texture.display();
			sf::Texture tex = texture.getTexture();
			sf::Image image = tex.copyToImage();
			image.saveToFile(std::string("out/hoshino_") + std::string(anim->getName().buffer()) + std::string(".png"));
		}
		printf("\n");
	}
	{
		auto vec = skeletonData->getSkins();
		for (int i = 0; i < vec.size(); ++i) {
			auto* skin = vec[i];
			printf("%s\n", skin->getName().buffer());
		}
		printf("\n");
	}

	drawable.state->setAnimation(0, "17", true);

	sf::RenderTexture texture;
	texture.create(c - a, d - b);
	texture.clear(sf::Color::Transparent);
	//sf::View view = texture.getDefaultView();
	//view.setViewport({ 0.262, 0.059, 0.5, 0.227 });
	texture.setView(sf::View{ {a, b, c - a, d - b} });
	texture.draw(drawable);
	texture.display();
	sf::Texture tex = texture.getTexture();
	sf::Image image = tex.copyToImage();
	image.saveToFile("out/test5.png");

	sf::RenderWindow window(sf::VideoMode(c - a, d - b), "Spine SFML - Hoshino");
	window.setFramerateLimit(60);
	sf::Event event;
	sf::Clock deltaClock;
	window.setView(sf::View{ {a, b, c - a, d - b} });

	while (window.isOpen()) {
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) window.close();

		float delta = deltaClock.getElapsedTime().asSeconds();
		deltaClock.restart();
		drawable.update(delta);
		window.clear(sf::Color::Transparent);
		window.draw(drawable);
		window.display();
	}
}

//DebugExtension dbgExtension(SpineExtension::getInstance());

int main () {
	//SpineExtension::setInstance(&dbgExtension);

	const char* binaryName = "data/hoshino_spr.skel";
	const char* atlasName = "data/hoshino_spr.atlas";
	float scale = 1.0f;

	SFMLTextureLoader textureLoader;
	auto atlas = make_unique<Atlas>(atlasName, &textureLoader);

	auto skeletonData = readSkeletonBinaryData(binaryName, atlas.get(), scale);
	hoshino(skeletonData.get(), atlas.get(), scale);

	//dbgExtension.reportLeaks();
	return 0;
}

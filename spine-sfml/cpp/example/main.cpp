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
#include <filesystem>
#include <set>
#include <spine/spine-sfml.h>
#include <spine/Debug.h>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace spine;
using namespace filesystem;
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

void exportSprite(SkeletonData* skeletonData, Atlas* atlas, float scale, std::string outpath) {
	SP_UNUSED(atlas);
	float w = skeletonData->getWidth() * scale;
	float h = skeletonData->getHeight() * scale;
	float x = skeletonData->getX() * scale;
	float y = skeletonData->getY() * scale;
	float a = floor(x), b = floor(-h - y), c = ceil(x + w), d = ceil(-y);
	//printf("%f %f %f %f\n", w, h, x, y);

	SkeletonDrawable drawable(skeletonData);
	drawable.timeScale = 1;
	drawable.setUsePremultipliedAlpha(true);

	Skeleton* skeleton = drawable.skeleton;
	skeleton->setPosition(0, 0);
	skeleton->updateWorldTransform();
	sf::RenderTexture texture;
	texture.create(c - a, d - b);
	sf::FloatRect bounds = {a, b, c - a, d - b}, prevBounds = bounds;
	texture.setView(sf::View{ bounds });

	auto skins = skeletonData->getSkins();
	for (int sid = 0; sid < skins.size(); ++sid) {
		auto* skin = skins[sid];
		string skinName = skin->getName().buffer();

		if (skinName != "default") {
			skeleton->setSkin(skin);
			// drawable.state->apply(*skeleton);
			// skeleton->updateWorldTransform();
		}

		auto vec = skeletonData->getAnimations();
		for (int i = 0; i < vec.size(); ++i) {
			auto* anim = vec[i];

			std::string name = anim->getName().buffer();

			// printf("%s\n", name.c_str());

			//size_t pos = name.find('_');
			//if (name.substr(0, pos) == "Eye" || name.substr(0, pos) == "Idle") {
			//	continue;
			//}
			//if (name.length() != 5 || !isdigit(name[3]) || !isdigit(name[4]) || name.substr(0, 3) != "S2_") {
			//	continue;
			//}
			//if (name.length() != 2 || !isdigit(name[0]) || !isdigit(name[1])) {
			//	continue;
			//}
			if (bool normal = name.length() == 2 && isdigit(name[0]) && isdigit(name[1]),
				mask = name.length() == 5 && isdigit(name[3]) && isdigit(name[4]) && name.substr(0, 3) == "S2_";
				!normal && !mask) {
				continue;
			}

			if (skinName != "default") name = skinName + "_" + name;
			// else continue;

			printf("%s\n", name.c_str());

			drawable.state->setAnimation(0, anim, true);
			drawable.update(0.0f);

			texture.clear(sf::Color::Transparent);
			drawable.draw(texture, sf::RenderStates::Default, &bounds);
			bounds = {floor(bounds.left), floor(bounds.top),
				ceil(bounds.left + bounds.width) - floor(bounds.left),
				ceil(bounds.top + bounds.height) - floor(bounds.top)};
			if (bounds != prevBounds) {
				texture.create(bounds.width, bounds.height);
				texture.setView(sf::View{ bounds });
				texture.clear(sf::Color::Transparent);
				texture.draw(drawable);
				prevBounds = bounds;
			}
			texture.display();

			const sf::Texture& tex = texture.getTexture();
			const sf::Image& image = tex.copyToImage();
			std::string outfile = outpath + name + ".png";
			image.saveToFile(outfile);
			system(("magick \"" + outfile + "\" -channel RGB -fx u/a -quality 55 \"" + outfile + "\"").c_str());
			//system(("magick \"" + outfile + "\" -quality 95 \"" + outfile + "\"").c_str());
			//system(("optipng -zc9 -zm9 -zs1 -f5 -nx -quiet	\"" + outfile + "\"").c_str());
			//system(("del \"" + outfile + "\"").c_str());
			//break;
		}
	}
	printf("\n");
	/*
	{
		auto vec = skeletonData->getSkins();
		for (int i = 0; i < vec.size(); ++i) {
			auto* skin = vec[i];
			printf("%s\n", skin->getName().buffer());
		}
		printf("\n");
	}*/

	/*
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
	image.saveToFile("out/test5.png");*/
	/*
	drawable.state->setAnimation(0, "Idle_01", true);

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
		window.clear(sf::Color::White);
		window.draw(drawable);
		window.display();
	}*/
}

void exportL2d(SkeletonData* skeletonData, Atlas* atlas, float scale, std::string outpath) {
	SP_UNUSED(atlas);
	float w = skeletonData->getWidth() * scale;
	float h = skeletonData->getHeight() * scale;
	float x = skeletonData->getX() * scale;
	float y = skeletonData->getY() * scale;
	float a = floor(x), b = floor(-h - y), c = ceil(x + w), d = ceil(-y);
	printf("%f %f %f %f\n", w, h, x, y);

	SkeletonDrawable drawable(skeletonData);
	drawable.timeScale = 1;
	drawable.setUsePremultipliedAlpha(true);

	Skeleton* skeleton = drawable.skeleton;
	skeleton->setPosition(0, 0);
	skeleton->updateWorldTransform();
	sf::RenderTexture texture;
	texture.create(c - a, d - b);
	texture.setView(sf::View{ {a, b, c - a, d - b} });

	auto vec = skeletonData->getAnimations();
	for (int i = 0; i < vec.size(); ++i) {
		auto* anim = vec[i];

		std::string name = anim->getName().buffer();

		//size_t pos = name.find('_');
		//if (name.substr(0, pos) == "Eye" || name.substr(0, pos) == "Idle") {
		//	continue;
		//}
		//if (name.length() != 5 || !isdigit(name[3]) || !isdigit(name[4]) || name.substr(0, 3) != "S2_") {
		//	continue;
		//}
		//if (name.length() != 2 || !isdigit(name[0]) || !isdigit(name[1])) {
		//	continue;
		//}
		if (name.substr(0, 5) != "Idle_") {
			continue;
		}

		printf("%s\n", name.c_str());

		drawable.state->setAnimation(0, anim, true);
		drawable.update(0.0f);

		texture.clear(sf::Color::Transparent);
		texture.draw(drawable);
		texture.display();

		const sf::Texture& tex = texture.getTexture();
		const sf::Image& image = tex.copyToImage();
		std::string outfile = outpath + name.c_str() + ".png";
		image.saveToFile(outfile);
		system(("magick \"" + outfile + "\" -channel RGB -fx u/a -quality 95 \"" + outfile + "\"").c_str());
		//system(("magick \"" + outfile + "\" -quality 95 \"" + outfile + "\"").c_str());
		//system(("optipng -zc9 -zm9 -zs1 -f5 -nx -quiet	\"" + outfile + "\"").c_str());
		//system(("del \"" + outfile + "\"").c_str());
		//break;
	}
	printf("\n");
	/*
	{
		auto vec = skeletonData->getSkins();
		for (int i = 0; i < vec.size(); ++i) {
			auto* skin = vec[i];
			printf("%s\n", skin->getName().buffer());
		}
		printf("\n");
	}*/

	/*
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
	image.saveToFile("out/test5.png");*/
	
	/*drawable.state->setAnimation(0, "Idle_01", true);

	sf::RenderWindow window(sf::VideoMode(c - a, d - b), "Spine SFML");
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
	}*/
}

void scales(SkeletonData* skeletonData, Atlas* atlas, float scale, std::string outpath) {
	SP_UNUSED(atlas);
	float w = skeletonData->getWidth() * scale;
	float h = skeletonData->getHeight() * scale;
	float x = skeletonData->getX() * scale;
	float y = skeletonData->getY() * scale;
	float margin = 0;
	float a = floor(x), b = floor(-h - y + margin), c = ceil(x + w), d = ceil(-y);
	printf("%f %f %f %f\n", w, h, x, y);
	printf("%f %f %f %f\n", a, b, c, d);

	SkeletonDrawable drawable(skeletonData);
	drawable.timeScale = 1;
	drawable.setUsePremultipliedAlpha(false);

	Skeleton* skeleton = drawable.skeleton;
	skeleton->setPosition(0, 0);
	skeleton->updateWorldTransform();
	sf::RenderTexture texture;
	texture.create(c - a, d - b);
	texture.setView(sf::View{ {a, b, c - a, d - b} });
	drawable.state->setAnimation(0, "00", true);

	drawable.update(0.0f);

	texture.clear(sf::Color::Transparent);
	texture.draw(drawable);
	texture.display();
	std::string outfile = outpath + to_string(scale) + ".png";

	const sf::Texture& tex = texture.getTexture();
	sf::Image image = tex.copyToImage();
	image.saveToFile(outfile);
	//system(("magick \"" + outfile + "\" -channel RGB -fx u/a -quality 95 \"" + outfile + "\"").c_str());
	system(("magick \"" + outfile + "\" -quality 95 \"" + outfile + "\"").c_str());
}

//DebugExtension dbgExtension(SpineExtension::getInstance());

int doall(int argc, const char* argv[]) {
	auto start = clock();
	if (argc < 4) {
		printf("arguments: <scale> <input> <output>\n");
		return 1;
	}

	//SpineExtension::setInstance(&dbgExtension);
	float scale = stof(argv[1]); //0.4375f
	path indir = argv[2]; //C:\games\ba\blue-archive-spine\assets\spine-pma
	path outroot = argv[3];
	//std::set<std::string> whitelist = { "ch0227" };
	std::set<std::string> whitelist(argv + 4, argv + argc);

	if (!exists(outroot)) {
		create_directory(outroot);
	}

	for (const auto& dir_entry : directory_iterator{ indir })
	{
		const auto& dir = dir_entry.path();
		string name = dir.filename().string();

		if (name.substr(name.size() - 4) != "_spr") {
			continue;
		}
		string outname = name.substr(0, name.size() - 4);
		path outdir = outroot / outname;
		string outfmt = (outdir / (outname + "_")).string();

		if (!whitelist.empty() && whitelist.find(outname) == whitelist.end()) {
			continue;
		}

		path binaryName = dir / (name + ".skel");
		path atlasName = dir / (name + ".atlas");
		//printf("Starting %s\n", outname.c_str());

		SFMLTextureLoader textureLoader;
		auto atlas = make_unique<Atlas>(atlasName.string().c_str(), &textureLoader);
		//printf("Loaded atlas\n");

		auto skeletonData = readSkeletonBinaryData(binaryName.string().c_str(), atlas.get(), scale);
		//printf("Loaded skeleton\n");

		if (!skeletonData->findAnimation("00")) {
			continue;
		}

		//if (exists(outdir)) {
		//	continue;
		//}

		printf("Starting %s\n", outname.c_str());
		create_directory(outdir);

		exportSprite(skeletonData.get(), atlas.get(), scale, outfmt);
	}

	printf("Took %f seconds\n", static_cast<float>(clock() - start) / CLOCKS_PER_SEC);

	//dbgExtension.reportLeaks();
	return 0;
}

int dol2d(int argc, const char* argv[]) {
	if (argc < 3) {
		printf("arguments: <scale> <input> <output>\n");
		return 1;
	}

	//SpineExtension::setInstance(&dbgExtension);
	float scale = stof(argv[1]); // 0.375
	path indir = argv[2];
	path outroot = argv[3];

	for (const auto& dir_entry : directory_iterator{ indir })
	{
		const auto& dir = dir_entry.path();
		string name = dir.filename().string();

		if (name.substr(name.size() - 5) != "_home") {
			continue;
		}

		string name2;
		for (const auto& subdir_entry : directory_iterator{ dir }) {
			const auto& f = subdir_entry.path();
			if (f.extension() == ".skel") {
				name2 = f.stem().string();
			}
		}

		string outname = name.substr(0, name.size() - 5);
		path outdir = outroot / outname;
		string outfmt = (outdir / (outname + "_")).string();

		//if (outname != "yuuka") {
		//	continue;
		//}

		path binaryName = dir / (name2 + ".skel");
		path atlasName = dir / (name2 + ".atlas");
		//printf("Starting %s\n", outname.c_str());

		SFMLTextureLoader textureLoader;
		auto atlas = make_unique<Atlas>(atlasName.string().c_str(), &textureLoader);
		//printf("Loaded atlas\n");

		auto skeletonData = readSkeletonBinaryData(binaryName.string().c_str(), atlas.get(), scale);
		//printf("Loaded skeleton\n");

		if (!skeletonData->findAnimation("Idle_01")) {
			continue;
		}

		if (exists(outdir)) {
			continue;
		}

		printf("Starting %s\n", outname.c_str());
		create_directory(outdir);

		exportL2d(skeletonData.get(), atlas.get(), scale, outfmt);
		//break;
	}

	//dbgExtension.reportLeaks();
	return 0;
}

int doscales(int argc, const char* argv[]) {
	if (argc < 2) {
		printf("arguments: <output>\n");
		return 1;
	}

	//doall(argc, argv);
	path indir = R"(C:\games\ba\blue-archive-spine\assets\spine\)";
	path outroot = argv[1];

	string name = "zunko_spr";
	path dir = indir / name;

	string outname = name.substr(0, name.size() - 4);
	path outdir = outroot / outname;
	create_directories(outdir);
	string outfmt = (outdir / (outname + "_")).string();

	path binaryName = dir / (name + ".skel");
	path atlasName = dir / (name + ".atlas");
	//printf("Starting %s\n", outname.c_str());

	SFMLTextureLoader textureLoader;
	auto atlas = make_unique<Atlas>(atlasName.string().c_str(), &textureLoader);
	//printf("Loaded atlas\n");

	for (float scale = 1.0f / 3.0f; scale <= 1.0f / 3.0f; scale += 0.03125f) {
		auto skeletonData = readSkeletonBinaryData(binaryName.string().c_str(), atlas.get(), scale);
		//printf("Loaded skeleton\n");

		printf("Starting %f\n", scale);

		scales(skeletonData.get(), atlas.get(), scale, outfmt);
	}

	return 0;
}

int premul(int argc, const char* argv[]) {
	if (argc < 3) {
		printf("arguments: <indir> <outdir>\n");
		return 1;
	}
	path indir = argv[1];
	path outdir = argv[2];

	if (!is_directory(outdir))
		create_directory(outdir);

	for (const auto& dir_entry : directory_iterator{ indir })
	{
		const auto& insubdir = dir_entry.path();
		string name = insubdir.filename().string();
		const auto& outsubdir = outdir / insubdir.filename();

		if (!is_directory(outsubdir))
			create_directory(outsubdir);

		for (const auto& subdir_entry : directory_iterator{ insubdir }) {
			const auto& inf = subdir_entry.path();
			const auto& outf = outsubdir / inf.filename();
			if (inf.extension() == ".png") {
				system(("magick \"" + inf.string() + "\" -channel RGB -fx u*a -quality 95 \"" + outf.string() + "\"").c_str());
				printf("%s\n", inf.string().c_str());
			} else {
				copy_file(inf, outf);
			}
		}
	}
	return 0;
}

int main(int argc, const char* argv[]) {
	if (!strcmp(argv[1], "premul")) return premul(argc - 1, argv + 1);
	if (!strcmp(argv[1], "sprites")) return doall(argc - 1, argv + 1);
	if (!strcmp(argv[1], "lobby")) return dol2d(argc - 1, argv + 1);
	return 1;
}

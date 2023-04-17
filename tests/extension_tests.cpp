#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fastgltf_parser.hpp>

#include "gltf_path.hpp"

TEST_CASE("Loading KHR_texture_basisu glTF files", "[gltf-loader]") {
    auto stainedLamp = sampleModels / "2.0" / "StainedGlassLamp" / "glTF-KTX-BasisU";

    auto jsonData = std::make_unique<fastgltf::GltfDataBuffer>();
    REQUIRE(jsonData->loadFromFile(stainedLamp / "StainedGlassLamp.gltf"));

    SECTION("Loading KHR_texture_basisu") {
        fastgltf::Parser parser(fastgltf::Extensions::KHR_texture_basisu);
        auto stainedGlassLamp = parser.loadGLTF(jsonData.get(), path, fastgltf::Options::DontRequireValidAssetMember);
        REQUIRE(parser.getError() == fastgltf::Error::None);
        REQUIRE(stainedGlassLamp != nullptr);

        REQUIRE(stainedGlassLamp->parse(fastgltf::Category::Textures | fastgltf::Category::Images) == fastgltf::Error::None);
        REQUIRE(stainedGlassLamp->validate() == fastgltf::Error::None);

        auto asset = stainedGlassLamp->getParsedAsset();
        REQUIRE(asset->textures.size() == 19);
        REQUIRE(!asset->images.empty());

        auto& texture = asset->textures[1];
        REQUIRE(texture.imageIndex == 1);
        REQUIRE(texture.samplerIndex == 0);
        REQUIRE(!texture.fallbackImageIndex.has_value());

        auto& image = asset->images.front();
        auto* filePath = std::get_if<fastgltf::sources::URI>(&image.data);
        REQUIRE(filePath != nullptr);
        REQUIRE(filePath->uri.valid());
        REQUIRE(filePath->uri.isLocalPath());
        REQUIRE(filePath->mimeType == fastgltf::MimeType::KTX2);
    }

    SECTION("Testing requiredExtensions") {
        // We specify no extensions, yet the StainedGlassLamp requires KHR_texture_basisu.
        fastgltf::Parser parser(fastgltf::Extensions::None);
        auto stainedGlassLamp = parser.loadGLTF(jsonData.get(), path, fastgltf::Options::DontRequireValidAssetMember);
        REQUIRE(stainedGlassLamp->parse() == fastgltf::Error::MissingExtensions);
    }
}

TEST_CASE("Loading KHR_texture_transform glTF files", "[gltf-loader]") {
    auto transformTest = sampleModels / "2.0" / "TextureTransformMultiTest" / "glTF";

    auto jsonData = std::make_unique<fastgltf::GltfDataBuffer>();
    REQUIRE(jsonData->loadFromFile(transformTest / "TextureTransformMultiTest.gltf"));

    fastgltf::Parser parser(fastgltf::Extensions::KHR_texture_transform);
    auto test = parser.loadGLTF(jsonData.get(), transformTest, fastgltf::Options::DontRequireValidAssetMember);
    REQUIRE(parser.getError() == fastgltf::Error::None);
    REQUIRE(test != nullptr);

    REQUIRE(test->parse(fastgltf::Category::Materials) == fastgltf::Error::None);
    REQUIRE(test->validate() == fastgltf::Error::None);

    auto asset = test->getParsedAsset();
    REQUIRE(!asset->materials.empty());

    auto& material = asset->materials.front();
    REQUIRE(material.pbrData.has_value());
    REQUIRE(material.pbrData->baseColorTexture.has_value());
    REQUIRE(material.pbrData->baseColorTexture->transform != nullptr);
    REQUIRE(material.pbrData->baseColorTexture->transform->uvOffset[0] == 0.705f);
    REQUIRE(material.pbrData->baseColorTexture->transform->rotation == Catch::Approx(1.5707963705062866f));
}

TEST_CASE("Test KHR_lights_punctual", "[gltf-loader]") {
    auto lightsLamp = sampleModels / "2.0" / "LightsPunctualLamp" / "glTF";
    fastgltf::GltfDataBuffer jsonData;
    REQUIRE(jsonData.loadFromFile(lightsLamp / "LightsPunctualLamp.gltf"));

    fastgltf::Parser parser(fastgltf::Extensions::KHR_lights_punctual);
    auto model = parser.loadGLTF(&jsonData, lightsLamp);
    REQUIRE(parser.getError() == fastgltf::Error::None);
    REQUIRE(model->parse(fastgltf::Category::Nodes) == fastgltf::Error::None);
    REQUIRE(model->validate() == fastgltf::Error::None);

    auto asset = model->getParsedAsset();
    REQUIRE(asset->lights.size() == 5);
    REQUIRE(asset->nodes.size() > 4);

    auto& nodes = asset->nodes;
    REQUIRE(nodes[3].lightsIndex.has_value());
    REQUIRE(nodes[3].lightsIndex.value() == 0);

    auto& lights = asset->lights;
    REQUIRE(lights[0].name == "Point");
    REQUIRE(lights[0].type == fastgltf::LightType::Point);
    REQUIRE(lights[0].intensity == 15.0f);
    REQUIRE(glm::epsilonEqual(lights[0].color[0], 1.0f, glm::epsilon<float>()));
    REQUIRE(glm::epsilonEqual(lights[0].color[1], 0.63187497854232788f, glm::epsilon<float>()));
    REQUIRE(glm::epsilonEqual(lights[0].color[2], 0.23909975588321689f, glm::epsilon<float>()));
}

TEST_CASE("Test KHR_materials_specular", "[gltf-loader]") {
    auto specularTest = sampleModels / "2.0" / "SpecularTest" / "glTF";
    fastgltf::GltfDataBuffer jsonData;
    REQUIRE(jsonData.loadFromFile(specularTest / "SpecularTest.gltf"));

    fastgltf::Parser parser(fastgltf::Extensions::KHR_materials_specular);
    auto model = parser.loadGLTF(&jsonData, specularTest);
    REQUIRE(model->parse(fastgltf::Category::Materials) == fastgltf::Error::None);
    REQUIRE(model->validate() == fastgltf::Error::None);

    auto asset = model->getParsedAsset();
    REQUIRE(asset->materials.size() >= 12);

    auto& materials = asset->materials;
    REQUIRE(materials[1].specular != nullptr);
    REQUIRE(materials[1].specular->specularFactor.has_value());
    REQUIRE(materials[1].specular->specularFactor == 0.0f);

    REQUIRE(materials[2].specular != nullptr);
    REQUIRE(materials[2].specular->specularFactor.has_value());
    REQUIRE(glm::epsilonEqual(*materials[2].specular->specularFactor, 0.051269f, glm::epsilon<float>()));

    REQUIRE(materials[8].specular != nullptr);
    REQUIRE(materials[8].specular->specularColorFactor.has_value());
    REQUIRE(glm::epsilonEqual((*(materials[8].specular->specularColorFactor))[0], 0.051269f, glm::epsilon<float>()));
    REQUIRE(glm::epsilonEqual((*(materials[8].specular->specularColorFactor))[1], 0.051269f, glm::epsilon<float>()));
    REQUIRE(glm::epsilonEqual((*(materials[8].specular->specularColorFactor))[2], 0.051269f, glm::epsilon<float>()));

    REQUIRE(materials[12].specular != nullptr);
    REQUIRE(materials[12].specular->specularColorTexture.has_value());
    REQUIRE(materials[12].specular->specularColorTexture.value().textureIndex == 2);
}
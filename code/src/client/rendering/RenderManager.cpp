#include "RenderManager.h"
#include "boost/thread/thread.hpp"
#include "../../common/ConfigurationDataHandler.h"
#include "../managers/GameManager.h"

RenderManager* RenderManager::instance = nullptr;
boost::mutex RenderManager::instanceMutex;

RenderManager::RenderManager()
{
	auto screenSize = getWindowSize();
	cam = Cam(screenSize.x, screenSize.y,
			  CONFIG_FLOAT("data.rendering.camera.fieldOfView"),
			  CONFIG_FLOAT("data.rendering.camera.nearPlane"),
			  CONFIG_FLOAT("data.rendering.camera.farPlane"));

	fogOfWarOpacity = CONFIG_FLOAT("data.world.fogOfWar.opacity");
	fogOfWarInnerRadius = CONFIG_FLOAT("data.world.fogOfWar.innerRadius");
	fogOfWarOuterRadius = CONFIG_FLOAT("data.world.fogOfWar.outerRadius");

	gl::enableAlphaBlending();

	initializeFogOfWar();

	lightPos = Vec3f(
		CONFIG_FLOAT("data.rendering.light.position.x"),
		CONFIG_FLOAT("data.rendering.light.position.y"),
		CONFIG_FLOAT("data.rendering.light.position.z")
	);
}

RenderManager::~RenderManager()
{
}

RenderManager * const RenderManager::getInstance()
{
	instanceMutex.lock();
	if(instance == nullptr)
	{
		instance = new RenderManager();
	}
	instanceMutex.unlock();

	return instance;
}

void RenderManager::releaseInstance()
{
	instanceMutex.lock();
	if(instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
	instanceMutex.unlock();
}

void RenderManager::renderPhongShadedModel(string modelName,
										   Vec4f ambient,
										   Vec4f diffuse,
										   Vec4f specular,
										   float shininess)
{
	auto model = ASSET_MGR->getModel(modelName);
	renderPhongShadedModel(model, ambient,
						   diffuse, specular, shininess);
}

void RenderManager::renderPhongShadedModel(TriMesh& model,
										   Vec4f ambient,
										   Vec4f diffuse,
										   Vec4f specular,
										   float shininess)
{
	auto shader = ASSET_MGR->getShaderProg(string("phong"));

	shader.bind();

	shader.uniform("lightPos", cam.getProjectionMatrix() * cam.getModelViewMatrix() * Vec3f(cam.getEyePoint().xy(), 1000.));

	shader.uniform("ambientColor", ambient);
	shader.uniform("diffuseColor", diffuse);
	shader.uniform("specularColor", specular);
	shader.uniform("shininess", shininess);

	gl::pushModelView();
		gl::draw(model);
	gl::popModelView();

	shader.unbind();
}

void RenderManager::renderSlimeShadedModel(TriMesh& model,
							   Vec3f worldTranslation,
							   Vec4f ambient,
							   Vec4f diffuse,
							   Vec4f specular,
							   float shininess)
{
	auto shader = ASSET_MGR->getShaderProg(string("slime"));

	shader.bind();

	shader.uniform("lightPos", cam.getProjectionMatrix() * cam.getModelViewMatrix() * Vec3f(cam.getEyePoint().xy(), 1000.));

	shader.uniform("ambientColor", ambient);
	shader.uniform("diffuseColor", diffuse);
	shader.uniform("specularColor", specular);
	shader.uniform("shininess", shininess);
	shader.uniform("time", float(getElapsedSeconds()));
	shader.uniform("translation", worldTranslation);

	gl::pushModelView();
		gl::draw(model);
	gl::popModelView();

	shader.unbind();
}

void RenderManager::renderSlimeShadedModel(string modelName,
							   Vec3f worldTranslation,
							   Vec4f ambient,
							   Vec4f diffuse,
							   Vec4f specular,
							   float shininess)
{
	auto model = ASSET_MGR->getModel(modelName);
	renderSlimeShadedModel(model, worldTranslation,
							ambient, diffuse, specular, shininess);
}

void RenderManager::renderBarrier(float radius)
{
	string barrier = "barrier";
	auto shader = ASSET_MGR->getShaderProg(barrier);

	shader.bind();

	Vec4f ambient = Vec4f(0.2f, 0.2f, 0.22f, 1.f);
	Vec4f diffuse = Vec4f(0.3f, 0.3f, 0.34f, 1.f);
	Vec4f specular = Vec4f(0.f, 0.f, 0.f, 1.f);
	float shininess = 0.f;

	shader.uniform("lightPos", cam.getProjectionMatrix() * cam.getModelViewMatrix() * lightPos);

	shader.uniform("ambientColor", ambient);
	shader.uniform("diffuseColor", diffuse);
	shader.uniform("radius", radius);

	gl::pushModelView();
		drawSphere(Vec3f(0.f, 0.f, 0.f), radius, (int) radius);
	gl::popModelView();

	shader.unbind();
}

void RenderManager::renderStatic(float radius, StaticModificator::Type type, float opacity)
{
	string staticString = "static";
	auto shader = ASSET_MGR->getShaderProg(staticString);

	shader.bind();

	Vec4f ambient; 
	Vec4f diffuse;

	switch (type)
	{
	case StaticModificator::RADIOACTIVITY:
		ambient = Vec4f(0.3f, 0.3f, 0.15f, opacity);
		diffuse = Vec4f(0.75f, 0.6f, 0.12f, opacity);
		break;
	case StaticModificator::NUTRIENTSOIL:
		ambient = Vec4f(0.15f, 0.55f, 0.15f, opacity);
		diffuse = Vec4f(0.14f, 0.4f, 0.14f, opacity);
		break;
	}

	float shininess = 0.f;

	shader.uniform("lightPos", cam.getProjectionMatrix() * cam.getModelViewMatrix() * lightPos);

	shader.uniform("ambientColor", ambient);
	shader.uniform("diffuseColor", diffuse);
	shader.uniform("radius", radius);

	gl::pushModelView();
		gl::drawSphere(Vec3f(0.f, 0.f, 0.f), radius, (int) radius);
	gl::popModelView();

	shader.unbind();
}

void RenderManager::renderSkin(TriMesh& model,
							   Vec4f ambient,
							   Vec4f diffuse,
							   Vec4f specular,
							   float shininess)
{
	auto shader = ASSET_MGR->getShaderProg(string("skin"));

	shader.bind();

	shader.uniform("lightPos", cam.getProjectionMatrix() * cam.getModelViewMatrix() * lightPos);

	shader.uniform("ambientColor", ambient);
	shader.uniform("diffuseColor", diffuse);
	shader.uniform("specularColor", specular);
	shader.uniform("shininess", shininess);
	shader.uniform("time", float(getElapsedSeconds()));

	gl::pushModelView();
		gl::draw(model);
	gl::popModelView();

	shader.unbind();
}

void RenderManager::renderBlackShadedModel(string modelName)
{
	auto model = ASSET_MGR->getModel(string("poly"));
	renderBlackShadedModel(model);
}
	
void RenderManager::renderBlackShadedModel(TriMesh& model)
{
	Vec4f ambient = Vec4f(0.3f, 0.3f, 0.3f, 1.f);
	Vec4f diffuse = Vec4f(0.2f, 0.2f, 0.2f, 0.5f);
	Vec4f specular = Vec4f(0.1, 0.1, 0.1, 0.3);
	float shininess = 10.f;

	auto shader = ASSET_MGR->getShaderProg(string("poly"));

	shader.bind();

	shader.uniform("lightPos", cam.getProjectionMatrix() * cam.getModelViewMatrix() * Vec3f(cam.getEyePoint().xy(), 1000.));

	shader.uniform("ambientColor", ambient);
	shader.uniform("diffuseColor", diffuse);

	gl::pushModelView();
		gl::draw(model);
	gl::popModelView();

	shader.unbind();
}

void RenderManager::setUp3d()
{
	/// enable back-face culling
	/// to avoid artifacts
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	gl::pushMatrices();
		gl::setMatrices(cam);
}

void RenderManager::shutdown3d()
{
		drawFogOfWar();
	gl::popMatrices();

	/// disable back-face culling
	/// to make 2D graphics visible
	glDisable(GL_CULL_FACE);
}

void RenderManager::zoomToWorld()
{
	/// world radius = sin(camera's fov / 2) * camera's distance
	/// camera's distance = world radius / sin(camera's fov / 2)
	float worldRadius = CONFIG_FLOAT("data.world.radius");
	float camDistance = worldRadius / (float) sin(cam.getFov() / 2.f / 180.f * M_PI);

	cam.setPosition(Vec3f(0, 0, camDistance));
}

void RenderManager::initializeFogOfWar()
{
	fogOfWarLayer = TriMesh();

	fogOfWarLayer.appendVertex(Vec3f(-1.f,  1.f, -1.f)); /// top left
	fogOfWarLayer.appendVertex(Vec3f( 1.f,  1.f, -1.f)); /// top right
	fogOfWarLayer.appendVertex(Vec3f(-1.f, -1.f, -1.f)); /// bottom left
	fogOfWarLayer.appendVertex(Vec3f( 1.f, -1.f, -1.f)); /// bottom right

	fogOfWarLayer.appendTriangle(0, 2, 1);
	fogOfWarLayer.appendTriangle(1, 2, 3);
}

void RenderManager::drawFogOfWar() const
{
	auto fogOfWarShaderProgram = ASSET_MGR->getShaderProg(string("fogOfWar"));
	fogOfWarShaderProgram.bind();

	GAME_SCR->getContainerMutex().lock();

	auto& cellsExploring = GAME_SCR->getExploringCells();
	int numOfCells = cellsExploring.getSize();

	Vec2f * centers2D = new Vec2f[numOfCells];
	float * innerRadii2D = new float[numOfCells];
	float * outerRadii2D = new float[numOfCells];
		
	int numOfRelevantCells = 0;

	for (auto it = cellsExploring.begin(); it != cellsExploring.end(); ++it)
	{
		auto center3D = it->second->getPosition();
		auto center2D = cam.worldToScreen(center3D, getWindowWidth(), getWindowHeight());
			
		auto radius3D = it->second->getRadius();
		auto innerRadius2D = center2D.distance(cam.worldToScreen(center3D + Vec3f(radius3D + fogOfWarInnerRadius, 0, 0), getWindowWidth(), getWindowHeight()));
		auto outerRadius2D = center2D.distance(cam.worldToScreen(center3D + Vec3f(radius3D + fogOfWarInnerRadius + fogOfWarOuterRadius, 0, 0), getWindowWidth(), getWindowHeight()));

		auto left = center2D.x - outerRadius2D;
		auto right = center2D.x + outerRadius2D;
		auto top = center2D.y - outerRadius2D;
		auto bottom = center2D.y + outerRadius2D;

		bool intersectsViewport = true;

		if (
			right < 0 ||
			left > getWindowWidth() ||
			top > getWindowHeight() ||
			bottom < 0
		)
		{
			intersectsViewport = false;
		}

		/// only pass values to the shader
		/// if they affect the viewport
		if (intersectsViewport)
		{
			centers2D[numOfRelevantCells] = center2D;
			innerRadii2D[numOfRelevantCells] = innerRadius2D * innerRadius2D;
			outerRadii2D[numOfRelevantCells] = outerRadius2D * outerRadius2D;

			++numOfRelevantCells;
		}
	}

	GAME_SCR->getContainerMutex().unlock();

	fogOfWarShaderProgram.uniform("screenSize", Vec2f(getWindowWidth(), getWindowHeight()));
	fogOfWarShaderProgram.uniform("numOfCells", numOfRelevantCells);
	fogOfWarShaderProgram.uniform("centers2D", centers2D, numOfRelevantCells);
	fogOfWarShaderProgram.uniform("innerRadii2D", innerRadii2D, numOfRelevantCells);
	fogOfWarShaderProgram.uniform("outerRadii2D", outerRadii2D, numOfRelevantCells);
	fogOfWarShaderProgram.uniform("maxOpacity", fogOfWarOpacity);

	delete centers2D;
	delete innerRadii2D;
	delete outerRadii2D;

	gl::draw(fogOfWarLayer);

	fogOfWarShaderProgram.unbind();
}

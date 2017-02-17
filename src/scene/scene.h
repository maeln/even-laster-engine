#ifndef SCENE_H
#define SCENE_H

#include "texture.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal, tangent, binormal;
	glm::vec2 uv[8];
};

class Mesh {
public:
	Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) :
		vertices(vertices),
		indices(indices)
	{
	}

	const std::vector<Vertex> getVertices() const { return vertices; }
	const std::vector<uint32_t> getIndices() const { return indices; }

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class Material {
	Texture2D *albedoMap;
	glm::vec4 albedoColor;

	// TODO: these should be baked (shinyness)
	Texture2D *normalMap;
	Texture2D *specularMap;
};

class Model {
public:
	Model(const Mesh *mesh, const Material *material) :
		mesh(mesh),
		material(material)
	{
	}

private:
	const Mesh *mesh;
	const Material *material;
};

class Transform {
public:
	Transform() : parent(nullptr)
	{
	}

	void setParent(Transform *parent)
	{
		if (this->parent != nullptr) {
			this->parent = nullptr;
			unrooted();
		}

		this->parent = parent;

		if (parent != nullptr)
			rooted();
	}

	virtual glm::mat4 getAbsoluteMatrix() const
	{
		glm::mat4 absoluteMatrix = getLocalMatrix();

		const Transform *curr = getParent();
		while (nullptr != curr) {
			absoluteMatrix = curr->getLocalMatrix() * absoluteMatrix;
			curr = curr->getParent();
		}

		return absoluteMatrix;
	}

	const Transform *getRootTransform() const
	{
		const Transform *curr = this;
		while (nullptr != curr->parent)
			curr = curr->parent;

		return curr;
	}

	Transform *getParent() const { return parent; }
	virtual glm::mat4 getLocalMatrix() const = 0;

protected:
	virtual void rooted() {}
	virtual void unrooted() {}

private:
	Transform *parent;
};

class RootTransform : public Transform {
public:
	RootTransform() : Transform()
	{
	}

	glm::mat4 getLocalMatrix() const override
	{
		return glm::mat4(1);
	}
};

class MatrixTransform : public Transform {
public:
	MatrixTransform() : Transform()
	{
		localMatrix = glm::mat4(1);
	}

	glm::mat4 getLocalMatrix() const override
	{
		return localMatrix;
	}

	void setLocalMatrix(glm::mat4 localMatrix)
	{
		this->localMatrix = localMatrix;
	}

private:
	glm::mat4 localMatrix;
};

class Object {
public:
	Object(Model *model, Transform *transform) :
		model(model),
		transform(transform)
	{
		assert(model != nullptr);
		assert(transform != nullptr);
	}

	const Transform *getTransform() const { return transform; }

private:
	Model *model;
	Transform *transform;
};

class Scene {
public:
	Scene()
	{
		transforms.push_back(&rootTransform);
	}

	MatrixTransform *createMatrixTransform(Transform *parent = nullptr)
	{
		auto trans = new MatrixTransform();

		if (parent == nullptr)
			parent = &rootTransform;

		trans->setParent(parent);
		transforms.push_back(trans);
		return trans;
	}

	Object *createObject(Model *model, Transform *transform = nullptr)
	{
		if (transform == nullptr)
			transform = &rootTransform;

		assert(transform->getRootTransform() == &rootTransform);

		auto obj = new Object(model, transform);
		objects.push_back(obj);
		return obj;
	}

	const Transform *getRootTransform() const { return &rootTransform; }

	const std::list<Object*> &getObjects() const { return objects; }
	const std::list<Transform*> &getTransforms() const { return transforms; }

private:
	std::list<Transform*> transforms;
	std::list<Object*> objects;
	RootTransform rootTransform;
};


#endif // SCENE_H
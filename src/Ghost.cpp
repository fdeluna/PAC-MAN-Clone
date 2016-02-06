#include "Ghost.h"

Ghost::Ghost(SceneManager* sceneManager, std::string nodeName, std::string entName, std::string mesh, GraphVertex* vertex, PacMan* pacman, GhostState gState, GraphVertex* hHome)
{
	sceneNode = sceneManager->createSceneNode(nodeName);
	entity = sceneManager->createEntity(entName, mesh);
	sceneNode->setPosition(vertex->getData().getPosition());
	sceneNode->attachObject(entity);
	currentVertex = vertex;
	previousVertex = currentVertex;
	pacMan = pacman;
	finalVertex = pacman->getCurrentVertex();
	currentDirecction = Direcction::UP;
	state = gState;
	targetVertex = NULL;
	home = hHome;

}

Ghost::~Ghost()
{
	entity = NULL;
	sceneNode = NULL;
}


void Ghost::update(const Ogre::FrameEvent& evt)
{
	time += evt.timeSinceLastFrame;
	switch (state)
	{
	case GhostState::WAIT:
		wait();
		break;
	case GhostState::CHASE:
		if (time < 20)
		{
			chase(pacMan->getCurrentVertex());
		}
		else
		{
			state = GhostState::SCATTER;
			currentVertex = previousVertex;
			time = 0;
		}
		break;
	case GhostState::SCATTER:
		if (time < 6)
		{
			chase(home);
		}
		else
		{
			state = GhostState::CHASE;
			currentVertex = previousVertex;
			time = 0;
		}

		break;
	case GhostState::SCARED:
		if (time < 6)
		{
			chase(pacMan->getCurrentVertex());
		}
		else
		{
			state = GhostState::SCATTER;
			currentVertex = previousVertex;
			time = 0;
		}

		break;
	case GhostState::DEAD:
		chase(Scene::getSingletonPtr()->getGhostRespawn(0));
		if (currentVertex->getData().getIndex() == Scene::getSingletonPtr()->getGhostRespawn(0)->getData().getIndex())
		{
			state = GhostState::EXIT;
			currentVertex = previousVertex;
			time = 0;
		}

		break;
	case GhostState::EXIT:
		chase(Scene::getSingletonPtr()->getExit());
		if (currentVertex->getData().getIndex() == Scene::getSingletonPtr()->getExit()->getData().getIndex())
		{
			state = GhostState::SCATTER;
			currentVertex = previousVertex;
			time = 0;
		}
		break;
	default:
		break;
	}

	if (pacMan->scaredGhost() && state != GhostState::EXIT && state != GhostState::WAIT && state != GhostState::DEAD)
	{
		state = GhostState::SCARED;
		currentVertex = previousVertex;
		time = 0;		
	}

	if (pacMan->getCurrentVertex()->getData().getIndex() == currentVertex->getData().getIndex())
	{
		state = GhostState::DEAD;
		pacMan->addScore(100);
		currentVertex = previousVertex;
		time = 0;
	}
}

void Ghost::wait()
{
	speed = 0.3;
	if (targetVertex && sceneNode->getPosition().distance(targetVertex->getData().getPosition()) > EPSILON)
	{
		sceneNode->translate(direcction * speed);
	}
	else
	{

		if (targetVertex)
		{
			currentVertex = targetVertex;
		}

		targetVertex = GraphVertex::nextVertx(getOppositeDirecction(currentDirecction), currentVertex);
		getDirecction(targetVertex, currentVertex);
	}
}



void Ghost::chase(GraphVertex* objective)
{
	if (targetVertex && sceneNode->getPosition().distance(targetVertex->getData().getPosition()) > EPSILON)
	{
		sceneNode->translate(direcction * speed);
	}
	else
	{
		speed = 0.75;
		if (targetVertex)
		{
			previousVertex = currentVertex;
			currentVertex = targetVertex;
		}
		if (currentVertex->getData().getIndex() == Scene::getSingletonPtr()->getRightTeleport()->getData().getIndex())
		{

			currentVertex = Scene::getSingletonPtr()->getLeftTeleport();
			speed = 0.4;
		}
		else if (currentVertex->getData().getIndex() == Scene::getSingletonPtr()->getLeftTeleport()->getData().getIndex())
		{
			currentVertex = Scene::getSingletonPtr()->getRightTeleport();
			speed = 0.4;
		}

		if (targetVertex && GraphVertex::checkVertex(targetVertex, "Teleport"))
		{
			speed = 0.4;
		}
		sceneNode->setPosition(currentVertex->getData().getPosition());

		if (state != GhostState::SCARED)
		{
			targetVertex = closerNextVertx(objective, currentVertex, previousVertex);
		}
		else
		{
			speed = 0.4;
			targetVertex = futherNextVertx(objective, currentVertex, previousVertex);
		}


		getDirecction(targetVertex, currentVertex);

	}
}

const bool Ghost::isDead()
{
	return false;
}

float Ghost::getSpeed()
{
	return speed;
}

void Ghost::setSpeed(float fSpeed)
{
	speed = fSpeed;
}

void  Ghost::setDirecction(Direcction dDirecction)
{
	currentDirecction = dDirecction;
}

void Ghost::getDirecction(GraphVertex* targetVertx, GraphVertex* actualVertx)
{

	if (actualVertx->getData().getPosition().z < targetVertx->getData().getPosition().z)
	{
		direcction = MOVE_UP;
		currentDirecction = Direcction::UP;
	}


	if (actualVertx->getData().getPosition().z > targetVertx->getData().getPosition().z)
	{
		direcction = MOVE_DOWN;
		currentDirecction = Direcction::DOWN;
	}

	if (actualVertx->getData().getPosition().x < targetVertx->getData().getPosition().x)
	{
		direcction = MOVE_RIGHT;
		currentDirecction = Direcction::RIGHT;
	}

	if (actualVertx->getData().getPosition().x > targetVertx->getData().getPosition().x)
	{
		currentDirecction = Direcction::LEFT;
		direcction = MOVE_LEFT;
	}
}



GraphVertex* Ghost::closerNextVertx(GraphVertex* targetVertx, GraphVertex* actualVertx, GraphVertex* previousVertex)
{
	GraphVertex *nextVertex = NULL;
	float closer = 100000000000000;
	if (actualVertx){
		for (GraphEdge* e : actualVertx->getEdges())
		{
			if (e->getDestination()->getData().getIndex() != previousVertex->getData().getIndex())
			{
				if (state == GhostState::DEAD || state == GhostState::EXIT || !GraphVertex::checkVertex(e->getDestination(), "CageDoor")){
					if (e->getDestination()->getData().getPosition().distance(targetVertx->getData().getPosition()) < closer){
						closer = e->getDestination()->getData().getPosition().distance(targetVertx->getData().getPosition());
						nextVertex = e->getDestination();
					}
				}
			}
		}
	}
	return nextVertex;
}



GraphVertex* Ghost::futherNextVertx(GraphVertex* targetVertx, GraphVertex* actualVertx, GraphVertex* previousVertex)
{
	GraphVertex *nextVertex = NULL;
	float futher = 0;
	if (actualVertx){
		for (GraphEdge* e : actualVertx->getEdges())
		{
			if (e->getDestination()->getData().getIndex() != previousVertex->getData().getIndex() && !GraphVertex::checkVertex(e->getDestination(), "CageDoor"))
			{
				if (e->getDestination()->getData().getPosition().distance(targetVertx->getData().getPosition()) > futher)
				{
					futher = e->getDestination()->getData().getPosition().distance(targetVertx->getData().getPosition());
					nextVertex = e->getDestination();
				}
			}
		}
	}
	return nextVertex;
}

Direcction Ghost::getOppositeDirecction(Direcction dDirecction)
{
	Direcction result = Direcction::NONE;

	switch (dDirecction)
	{
	case Direcction::UP:
		result = Direcction::DOWN;
		break;
	case Direcction::DOWN:
		result = Direcction::UP;
		break;
	case Direcction::RIGHT:
		result = Direcction::LEFT;
		break;
	case Direcction::LEFT:
		result = Direcction::RIGHT;
		break;
	}

	return result;
}

//
//void Ghost::isScared()
//{
//	if (state != GhostState::EXIT && state != GhostState::WAIT && state != GhostState::DEAD)
//	{
//		state = GhostState::SCARED;
//		targetVertex = NULL;
//		time = 0;
//		previousVertex = currentVertex;
//	}
//}
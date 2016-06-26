#ifndef CONV_HULL
#define CONV_HULL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>

#include <time.h>
#include <stdlib.h>
#include <vector>
#include <chrono>
#include <math.h>


struct Edge {

	int p1;
	int p2;
	bool operator==(const Edge& rhs) {
		return (p1 == rhs.p1 && p2 == rhs.p2) 
			|| (p2 == rhs.p1 && p1 == rhs.p2) 
			|| (p1 == rhs.p2 && p2 == rhs.p1);
	}
};

glm::vec4 generateRandom2DPoint(glm::vec2 min, glm::vec2 max) {
	
	double randx = min.x + (max.x - min.x)*(1.0*rand() / RAND_MAX);
	double randy = min.y + (max.y - min.y)*(1.0*rand() / RAND_MAX);

	glm::vec4 r(randx, randy, 0.0, 1.0);
	return r;
}

glm::vec4 generateRandom3DPoint(glm::vec3 min, glm::vec3 max) {
	double randx = min.x + (max.x - min.x)*(1.0*rand() / RAND_MAX);
	double randy = min.y + (max.y - min.y)*(1.0*rand() / RAND_MAX);
	double randz = min.z + (max.z - min.z)*(1.0*rand() / RAND_MAX);

	glm::vec4 r(randx, randy, randz, 1.0);
	return r;
}

void generateRandom2DPoints(int N, std::vector<glm::vec4>& points, glm::vec2 min, glm::vec2 max) {
	srand (time(NULL));
	for(int i = 0; i < N; ++i) {
		points.push_back(generateRandom2DPoint(min, max));
	}
}

glm::vec4 generateRandom3DPoints(int N, std::vector<glm::vec4>& points, glm::vec3 min, glm::vec3 max) {
	srand (time(NULL));
	for(int i = 0; i < N; ++i) {
		points.push_back(generateRandom3DPoint(min, max));
	}
}


glm::vec4 minYPoint(std::vector<glm::vec4>& points) {
	glm::vec4 minPoint = points[0];
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].y < minPoint.y) {
			minPoint = points[i];
		}
	}
	return minPoint;
}

glm::vec4 maxYPoint(std::vector<glm::vec4>& points) {
	glm::vec4 minPoint = points[0];
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].y > minPoint.y) {
			minPoint = points[i];
		}
	}
	return minPoint;
}


glm::vec4 minXPoint(std::vector<glm::vec4>& points) {
	glm::vec4 minPoint = points[0];
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].x < minPoint.x) {
			minPoint = points[i];
		}
	}
	return minPoint;
}

glm::vec4 maxXPoint(std::vector<glm::vec4>& points) {
	glm::vec4 maxPoint = points[0];
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].x > maxPoint.x) {
			maxPoint = points[i];
		}
	}
	return maxPoint;
}

glm::vec4 minZPoint(std::vector<glm::vec4>& points) {
	glm::vec4 minPoint = points[0];
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].z < minPoint.z) {
			minPoint = points[i];
		}
	}
	return minPoint;
}

glm::vec4 maxZPoint(std::vector<glm::vec4>& points) {
	glm::vec4 maxPoint = points[0];
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].z > maxPoint.z) {
			maxPoint = points[i];
		}
	}
	return maxPoint;
}

int testSide(glm::vec4 pointOnHull, glm::vec4 currentPoint, glm::vec4 testPoint) {
	double det = (currentPoint.x - pointOnHull.x) * (testPoint.y - pointOnHull.y) - 
	(testPoint.x - pointOnHull.x) * (currentPoint.y - pointOnHull.y);
	if(det > 0) {
		return -1; //to the left
	}

	if(det < 0) {
		return 1; //to the right
	}

	return 0; //collinear
}

int testSide3D(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 test) {
	glm::vec3 p1_3(p1.x,p1.y,p1.z);
	glm::vec3 p2_3(p2.x,p2.y,p2.z);
	glm::vec3 p3_3(p3.x,p3.y,p3.z);
	glm::vec3 test_3(test.x, test.y, test.z);

	glm::vec3 B = p2_3 - p1_3;
	glm::vec3 C = p3_3 - p1_3;
	glm::vec3 X = test_3 - p1_3;

	//calculate 3x3 determinant
	double det = B.x*C.y*X.z + C.x*X.y*B.z + X.x*B.y*C.z -
		X.x*C.y*B.z - C.x*B.y*X.z - B.x*X.y*C.z;

	if(det > 0) {
		return 1;
	}

	if(det < 0) {
		return -1;
	}

	return 0;
}

void visualizeGiftWrapping(VAO& v, std::vector<glm::vec4>& hullPoints) {
	int k;
	for(k = 0; k < v.data->obj_vertices.size()-1; ++k) {
	  v.data->obj_lines.push_back(glm::uvec2(k, k+1));
	}
	v.data->obj_lines.push_back(glm::uvec2(k, 0));
	v.updateLines();
}


void visualizeBruteForce(VAO& v, std::vector<glm::vec4>& hullPoints) {
	int k;
	for(k = 0; k < v.data->obj_vertices.size()-1; k+=2) {
	  v.data->obj_lines.push_back(glm::uvec2(k, k+1));
	}
	v.updateLines();
}


std::vector<glm::vec4> giftWrapping(std::vector<glm::vec4>& points) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	std::vector<glm::vec4> hullPoints;

	if(points.size() < 4) {
		return points;
	}

	glm::vec4 h = minYPoint(points);
	do {
		hullPoints.push_back(h);
		glm::vec4 currentBest = points[0];

		for(int i = 1; i < points.size(); ++i) {
			if(h == currentBest || testSide(h, currentBest, points[i]) == -1) {
				currentBest = points[i];
			}
		}

		h = currentBest;
	} while(h != hullPoints[0]);
	//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  	//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<std::endl;

	return hullPoints;
	
}

glm::vec3 xyz(glm::vec4 p) { 
	return glm::vec3(p.x, p.y, p.z);
}

double distanceToLine(glm::vec4 point, glm::vec4 linePoint1, glm::vec4 linePoint2) {
	glm::vec3 unitLine = glm::normalize(xyz(linePoint2 - linePoint1));
	glm::vec3 ba = xyz(point - linePoint1);
	return glm::length(glm::cross(ba, unitLine));
}


void quickHull2DRecursive(std::vector<glm::vec4>& points, std::vector<glm::vec4>& hull, glm::vec4 p1, glm::vec4 p2) {
	//find point with max distance  from line
	glm::vec4 maxDistPoint = points[0];
	double maxDist = distanceToLine(maxDistPoint, p1, p2);
	for(int i = 1; i < points.size(); ++i) {
		double tmpD = distanceToLine(points[i], p1, p2);
		if(tmpD > maxDist) {
			maxDist = tmpD;
			maxDistPoint = points[i];
		}
	}

	std::vector<glm::vec4> above;
	std::vector<glm::vec4> below;
	for(int i = 0; i < points.size(); ++i) {
		if(points[i] == maxDistPoint) { continue; }

		int cr = testSide(p1, maxDistPoint, points[i]);
		if(cr > 0) {

			below.push_back(points[i]);
		}

		int cr2 = testSide(p2, maxDistPoint, points[i]);
		if(cr2 < 0) {
			above.push_back(points[i]);
		}
	}

	//std::cout << "points below: " << below.size() << " points above: " << above.size() << std::endl;

	if(below.size() > 0) {
		quickHull2DRecursive(below, hull, p1, maxDistPoint);
	}
	hull.push_back(maxDistPoint);

	if(above.size() > 0) {
		quickHull2DRecursive(above, hull, maxDistPoint, p2);
	}

}


std::vector<glm::vec4> quickHull2D(std::vector<glm::vec4> points) {
	std::vector<glm::vec4> hull;
	glm::vec4 minPointY = minYPoint(points);
	glm::vec4 maxPointY = maxYPoint(points);
	hull.push_back(minPointY);
	

	glm::vec3 vector = glm::normalize(xyz(maxPointY - minPointY));
	std::vector<glm::vec4> right;
	std::vector<glm::vec4> left;
	for(int i = 0; i < points.size(); ++i) {
		if(points[i] == minPointY || points[i] == maxPointY) { continue; }

		int cr = testSide(minPointY, maxPointY, points[i]);
		if(cr > 0) {
			//to the right of the line
			right.push_back(points[i]);
		} else {
			//to the left of the line
			left.push_back(points[i]);
		}
	}
	if(right.size() > 0) {
		quickHull2DRecursive(right, hull, minPointY, maxPointY);
	}

	hull.push_back(maxPointY);
	if(left.size() > 0) {
		quickHull2DRecursive(left, hull, maxPointY, minPointY);
	}

	return hull;

}

std::vector<glm::vec4> bruteForce2D(std::vector<glm::vec4> points) {
	std::vector<glm::vec4> hull;
	for(int i = 0; i < points.size(); ++i) {
		glm::vec4 pointI = points[i];
		for(int j = i; j < points.size(); ++j) {
			glm::vec4 pointJ = points[j];
			if(i == j) { continue; }

			int initPoint = 0;
			if(i == 0 || j == 0) {
				initPoint++;
				if(i == 1 || j == 1) {
					initPoint++;
				}

			}
			int side = testSide(pointI, pointJ, points[initPoint]);
			bool difSides = false;
			for(int k = 0; k < points.size(); ++k) {
				if(k != i && k != j) {
					if(testSide(pointI, pointJ, points[k]) != side) {
						difSides = true;
						break;
					}
				}
			}

			if(!difSides) {
				hull.push_back(points[i]);
				hull.push_back(points[j]);
			}
		}
	}

	return hull;
}

void bruteForceHull3D(std::vector<glm::vec4>& points, std::vector<glm::uvec3>& hullFaces) {

	for(int i = 0; i < points.size(); ++i) {
		glm::vec4 pi = points[i];
		for(int j = i; j < points.size(); ++j) {
			glm::vec4 pj = points[j];
			if(i == j) { continue;}
			for(int k = j; k < points.size(); ++k) {
				glm::vec4 pk = points[k];
				if(i == k || j == k) { continue;}
				int initPoint = 0;
				while(initPoint == i || initPoint == j || initPoint == k) {
					initPoint++;
				}
				int side = testSide3D(pi, pj, pk, points[initPoint]);

				bool difSides = false;
				for(int m = 0; m < points.size(); ++m) {
					if(m != i && m != j && m != k) {
						if(testSide3D(pi, pj, pk, points[m]) != side) {
							difSides = true;
							break;
						}
					}
				}

				if(!difSides) {
					hullFaces.push_back(glm::uvec3(i, j, k));
				}
			}
		}
	}
}


float dot(glm::vec3 a, glm::vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

float sineOfAngle(glm::vec4 A, glm::vec4 B, glm::vec4& D) {
	glm::vec3 bmina = xyz(B-A);
	glm::vec3 ab = glm::normalize(bmina);
	glm::vec3 d = xyz(D);
	return dot(ab, d);
}

Edge findInitialEdge(std::vector<glm::vec4>& points) {

	int min = 0;
	float minY = points[0].y;
	for(int i = 1; i < points.size(); ++i) {
		if(points[i].y < minY) {
			minY = points[i].y;
			min = i;
		}
	}

	int currentBest = 0;
	int h = min;

	for(int i = 1; i < points.size(); ++i) {
		if(h == currentBest || testSide(points[h], points[currentBest], points[i]) == -1) {
			currentBest = i;
		}
	}

	Edge e;
	e.p1 = min;
	e.p2 = currentBest;
	return e;


}

void giftWrapping3D(std::vector<glm::vec4>& points, std::vector<glm::uvec3>& hullFaces) {

	std::vector<Edge> todo;
	std::vector<Edge> edgeList;
	Edge initial = findInitialEdge(points);

	todo.push_back(initial);

	

	do {

		Edge edge = todo[todo.size()-1];
		todo.pop_back();


		int i = 0;
		while(i == edge.p1 || i == edge.p2) {
			++i;
		}


		int C = i;
		float maxDistance;
		float maxAbsDistance;

		do {

			glm::vec3 N = glm::normalize(glm::cross(xyz(points[edge.p1]) - xyz(points[edge.p2]), xyz(points[C]) - xyz(points[edge.p2])));

			maxDistance = -99999.0;
			maxAbsDistance = -99999.0;

			
			int maxP;
			for(int i = 0; i < points.size(); ++i) {
				glm::vec4 P = points[i];
				if(P == points[C] || P == points[edge.p1] || P == points[edge.p2]) { continue; }
				float d = dot((xyz(P) - xyz(points[edge.p2])), N);
				if(d > maxDistance) {
					maxAbsDistance = abs(d);
					maxDistance = d;
					maxP = i;
				}
			}

			if(maxDistance > 0) {
				C = maxP;
			}
		} while(maxDistance > 0);

		Edge e1;
		e1.p1 = edge.p2;
		e1.p2 = C;
		Edge e2;
		e2.p1 = C;
		e2.p2 = edge.p1;

		Edge e3;
		e3.p1 = edge.p1;
		e3.p2 = edge.p2;

		Edge tmp[3];
		tmp[0] = e1;
		tmp[1] = e2;
		tmp[2] = e3;

		hullFaces.push_back(glm::uvec3(edge.p2, C, edge.p1));

		for(int i = 0; i < 3; ++i){
			Edge e_prime;
			e_prime.p1 = tmp[i].p2;
			e_prime.p2 = tmp[i].p1;

			bool inEdgeList = false;
			for(int i = 0; i < edgeList.size(); ++i) {
				if(e_prime == edgeList[i]) {
					inEdgeList = true;
					for(int j = 0; j < todo.size(); ++j) {
						if(todo[j] == tmp[i]) {
							todo.erase(todo.begin()+j);
						}
					}
				}
			}

			if(!inEdgeList) {
				edgeList.push_back(tmp[i]);
				todo.push_back(e_prime);
			}
		}

	} while(todo.size() > 0);
}

#endif

#include <bingus.h>
#include <limits>

bool TestAABBAABB(const AABB& a, const AABB& b)
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	return true;
}

bool TestCircleCircle(const Circle& a, const Circle& b)
{
	vec2 d = a.position - b.position;
	float dist2 = glm::dot(d, d);
	float radiusSum = a.radius + b.radius;
	return dist2 <= glm::pow(radiusSum, 2);
}

float SqDistPointToAABB(vec2 point, AABB box)
{
	float sqDist = 0.f;

	if (point.x < box.min.x) sqDist += glm::pow(box.min.x - point.x, 2);
	if (point.x > box.max.x) sqDist += glm::pow(point.x - box.max.x, 2);

	if (point.y < box.min.y) sqDist += glm::pow(box.min.y - point.y, 2);
	if (point.y > box.max.y) sqDist += glm::pow(point.y - box.max.y, 2);

	return sqDist;
}

bool TestCircleAABB(const Circle& circle, const AABB& box)
{
	float sqDist = SqDistPointToAABB(circle.position, box);
	return sqDist <= glm::pow(circle.radius, 2);
}

bool IntersectRayAABB(const Ray& ray, const AABB& box, float& tmin, vec2& point)
{
	tmin = 0.f;
	float tmax = std::numeric_limits<float>::max();

	//X axis slab
	if (glm::abs(ray.direction.x) < std::numeric_limits<float>::epsilon())
	{
		//Ray is parallel to the slab. If the origin is not within it, there is no collision
		if (ray.start.x < box.min.x || ray.start.x > box.max.x) return false;
	}
	else
	{
		//Compute intersection t value of ray with near and far plane of slab
		float ood = 1.f / ray.direction.x;
		float t1 = (box.min.x - ray.start.x) * ood;
		float t2 = (box.max.x - ray.start.x) * ood;
		//Make t1 be intersection with near plane, t2 with far plane
		if (t1 > t2) std::swap(t1, t1);
		//Compute intersection of slab intersection intervals
		if (t1 > tmin) tmin = t1;
		if (t2 > tmax) tmax = t2;
		//No collision if slab intersection becomes empty
		if (tmin > tmax) return false;
	}

	//Y axis slab
	if (glm::abs(ray.direction.y) < std::numeric_limits<float>::epsilon())
	{
		//Ray is parallel to the slab. If the origin is not within it, there is no collision
		if (ray.start.y < box.min.y || ray.start.y > box.max.y) return false;
	}
	else
	{
		//Compute intersection t value of ray with near and far plane of slab
		float ood = 1.f / ray.direction.y;
		float t1 = (box.min.y - ray.start.y) * ood;
		float t2 = (box.max.y - ray.start.y) * ood;
		//Make t1 be intersection with near plane, t2 with far plane
		if (t1 > t2) std::swap(t1, t1);
		//Compute intersection of slab intersection intervals
		if (t1 > tmin) tmin = t1;
		if (t2 > tmax) tmax = t2;
		//No collision if slab intersection becomes empty
		if (tmin > tmax) return false;
	}

	//Ray intersects both slabs. Return intersection point and t value
	point = ray.start + ray.direction * tmin;
	return true;
}

bool IntersectRayCircle(const Ray& ray, const Circle& circle, vec2& p, float& t)
{
	vec2 m = ray.start - circle.position;
	float b = glm::dot(m, ray.direction);
	float c = glm::dot(m, m) - glm::pow(circle.radius, 2);

	//No intersection if the ray's origin is outside the circle (c > 0), and the ray points away from the circle (b > 0)
	if (c > 0.f && b > 0.f) return false;

	float discr = glm::pow(b, 2) - c;

	//No intersection if discriminant is negative
	if (discr < 0.f) return false;

	//Ray intersects sphere, compute smallest t value of intersection
	t = -b - glm::sqrt(discr);

	//If t is negative, ray start is inside sphere so clamp t to zero
	t = glm::max(t, 0.f);

	//Compute p from t and return
	p = ray.start + ray.direction * t;
	return true;
}

bool IntersectSegmentCircle(const Segment& segment, const Circle& circle, vec2& p, float& t)
{
	float m = glm::length(segment.end - segment.start);
	Ray r = Ray(segment.start, (segment.end - segment.start) / m);
	if (IntersectRayCircle(r, circle, p, t))
	{
		return t <= m;
	}
	return false;
}

vec2 Corner(const AABB& box, int n)
{
	assert(n >= 0 && n <= 3);
	if (n == 0) return box.min;
	if (n == 1) return vec2(box.max.x, box.min.y);
	if (n == 2) return box.max;
	if (n == 3) return vec2(box.min.x, box.max.y);
}

bool MovingCircleToAABB(const Circle& circle, vec2 velocity, const AABB& box, float& t)
{
	//Compute a box e, which is our AABB expanded by the circle radius
	AABB e = box;
	e.min -= vec2(circle.radius);
	e.max += vec2(circle.radius);

	//If the ray defined by the circle's movement does not collide with the expanded box e, there can be no collision.
	//If it does collide, get p and t
	vec2 p;
	Ray r = Ray(circle.position, velocity); //NOTE: Does this need to be normalized?
	if (!IntersectRayAABB(r, e, t, p) || t > 1.f) return false;

	//Compute whether p lies in an edge region or vertex region of our AABB
	int u = 0;
	int v = 0;
	if (p.x < box.min.x) u |= 1;
	if (p.x > box.max.x) v |= 1;
	if (p.y < box.min.y) u |= 2;
	if (p.y > box.max.y) v |= 2;

	//Or the bits together into a mask
	int m = u + v;

	//If both bits are set (m = 3) then p is in a vertex region
	if (m == 3)
	{
		//Must now intersect segment [c, c+d] against the capsules of the three edges
		//meeting at the vertex and return the best time, if one or more hit
		float tmin = std::numeric_limits<float>::min();

	}


	return false;
}
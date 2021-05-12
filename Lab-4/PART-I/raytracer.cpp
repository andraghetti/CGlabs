#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"

// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay (Ray & ray, Hit & h, bool use_sphere_patches) const
{
  bool answer = false;
  Hit nearest;
  nearest = Hit();

  // intersect each of the quads
  for(int i = 0; i < mesh->numQuadFaces(); i++)
  {
	Face *f = mesh->getFace (i);
	if (f->intersect (ray, h, args->intersect_backfacing))
	{
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	}
  }

  // intersect each of the spheres(either the patches, or the original spheres)
  if (use_sphere_patches)
  {
	for(int i = mesh->numQuadFaces(); i < mesh->numFaces(); i++)
	{
	  Face *f = mesh->getFace (i);
	  if (f->intersect (ray, h, args->intersect_backfacing))
	  {
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	  }
	}
  }
  else
  {
	const vector < Sphere * >&spheres = mesh->getSpheres();
	for(unsigned int i = 0; i < spheres.size (); i++)
	{
	  if (spheres[i]->intersect (ray, h))
	  {
		if( h.getT() < nearest.getT() )
		{
			answer = true;
			nearest = h;
		}
	  }
	}
  }

  h = nearest;
  return answer;
}

Vec3f RayTracer::TraceRay (Ray &ray, Hit &hit, int bounce_count) const
{
  hit = Hit(); //punto in cui colpisce
  bool intersect = CastRay (ray, hit, false); //crea il raggio e interseca

  //DEBUG
  if( bounce_count == args->num_bounces )
  	RayTree::SetMainSegment (ray, 0, hit.getT() );
  else
	RayTree::AddReflectedSegment(ray, 0, hit.getT() );

  //INIZIO
  Vec3f answer = args->background_color; //answer contiene il colore da restituire. Se non incontra altro, è il background

  if (intersect == true)
  {
	Material *m = hit.getMaterial(); //materiale dell'oggetto intersecato
	assert (m != NULL);

	Vec3f normal = hit.getNormal();
	float T = hit.getT(); //parametro rispetto al quale è avvenuta l'intersezione
	Vec3f point = ray.pointAtParameter(T); //vettore che parte dall'origine del raggio e arriva al punto di intersezione
	
	// se i raggi vengono da una sorgente luminosa, meglio mettere direttamente colore bianco senza calcolare altro
	if (m->getEmittedColor().Length() > 0.0001)
		return Vec3f(1, 1, 1);
	
										   
	// ambient light
	answer = args->ambient_light * m->getDiffuseColor(); //si mette in answer la luce ambiente.

	// ==========================================
	//			SHADOW LOGIC (CAST RAY)
	// ==========================================

	int shadowsSamples = args->num_shadow_samples; //default = 0

	int num_lights = mesh->getLights().size();

	for (int i = 0; i < num_lights; i++) //per ciascuna luce dobbiamo generare uno shadow ray
	{
		Face *currentLight = mesh->getLights()[i];

		if (currentLight->getArea() == 0 && shadowsSamples!=0) //luce puntiforme 
			shadowsSamples = 1;
		
		if (shadowsSamples > 0)
		{
			for (int j = 0; j < shadowsSamples; j++) //prendo tanti random point della luce, quanto specificato in args
			{
				Vec3f pointOnLight;

				if (shadowsSamples == 1)
					pointOnLight = currentLight->computeCentroid();
				else
					pointOnLight = currentLight->RandomPoint();

				Vec3f dirToLight = pointOnLight - point;
				dirToLight.Normalize();

				// crea shadow ray verso il punto luce
				Ray *shadowRay = new Ray(point, dirToLight);
				Hit *shadowHit = new Hit();

				// controlla il primo oggetto colpito da tale raggio
				if (CastRay(*shadowRay, *shadowHit, false))
				{
					RayTree::AddShadowSegment(*shadowRay, 0, shadowHit->getT()); //per DEBUG

					Vec3f hitPoint = shadowRay->pointAtParameter(shadowHit->getT());
					Vec3f dista = hitPoint - pointOnLight; //calcola il vettore distanza fra punto e punto luce
					if (dista.Length() < 0.001) //punto luce
					{
						if (normal.Dot3(dirToLight) > 0) //prodotto scalare tra i due vettori
						{
							Vec3f lightColor = 0.2 * currentLight->getMaterial()->getEmittedColor() * currentLight->getArea();
							answer += m->Shade(ray, hit, dirToLight, lightColor, args); //SHADE fa il calcolo di PHONG
						}
					}
				}
			}
			answer /= shadowsSamples;
		}
		else //no ombre
		{
			Vec3f pointOnLight = currentLight->computeCentroid();
			Vec3f dirToLight = pointOnLight - point;
			dirToLight.Normalize();

			Vec3f lightColor = 0.2 * currentLight->getMaterial()->getEmittedColor() * currentLight->getArea();
			answer += m->Shade(ray, hit, dirToLight, lightColor, args); //SHADE fa il calcolo di PHONG
		}
	}

	// ==========================================
	//			REFLECTIVE LOGIC
	// ==========================================
	Vec3f reflectiveColor = m->getReflectiveColor(); //se l'oggetto non è riflettente si fa solo il cast ray (Shadows)

	if (reflectiveColor.Length() != 0 && bounce_count > 0) //se reflectiveColor==0  allora l'oggetto non è riflettente -> no raycast
	{
		Vec3f rayVector = ray.getOrigin() - point;
		Vec3f reflection = (2 * normal.Dot3(rayVector) * normal) - rayVector; //calcola ReflectionRay  R=2<n, l> n - l
		reflection.Normalize();
		answer += TraceRay(Ray(point,reflection), hit, bounce_count - 1); //aggiunge ad answer il contributo riflesso che ritorna dalla ricorsione
		answer = answer * reflectiveColor; //	<-	questo è fondamentale (bisognerebbe definire il *= )
	}

  } // se non interseca -> solo background
  
  return answer;
}

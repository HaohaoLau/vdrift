/************************************************************************/
/*                                                                      */
/* This file is part of VDrift.                                         */
/*                                                                      */
/* VDrift is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* VDrift is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with VDrift.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#include "loaddrawable.h"
#include "content/contentmanager.h"
#include "graphics/texture.h"
#include "graphics/model.h"
#include "cfg/ptree.h"

#include <string>
#include <vector>

LoadDrawable::LoadDrawable(
	const std::string & path,
	const int anisotropy,
	ContentManager & content,
	std::set<std::tr1::shared_ptr<Model> > & models,
	std::set<std::tr1::shared_ptr<Texture> > & textures,
	std::ostream & error) :
	path(path),
	anisotropy(anisotropy),
	content(content),
	models(models),
	textures(textures),
	error(error)
{
	// ctor
}

bool LoadDrawable::operator()(
	const PTree & cfg,
	SceneNode & topnode,
	keyed_container<SceneNode>::handle * nodehandle,
	keyed_container<Drawable>::handle * drawhandle)
{
	std::vector<std::string> texname;
	if (!cfg.get("texture", texname)) return true;

	std::string meshname;
	if (!cfg.get("mesh", meshname, error)) return false;

	return operator()(meshname, texname, cfg, topnode, nodehandle, drawhandle);
}

bool LoadDrawable::operator()(
	const std::string & meshname,
	const std::vector<std::string> & texname,
	const PTree & cfg,
	SceneNode & topnode,
	keyed_container<SceneNode>::handle * nodeptr,
	keyed_container<Drawable>::handle * drawptr)
{
	Drawable drawable;

	// set textures
	std::tr1::shared_ptr<Texture> tex[3];
	TextureInfo texinfo;
	texinfo.mipmap = true;
	texinfo.anisotropy = anisotropy;
	if (texname.empty())
	{
		error << "No texture defined" << std::endl;
		return false;
	}
	else
	{
		content.load(tex[0], path, texname[0], texinfo);
		textures.insert(tex[0]);
	}
	if (texname.size() > 1)
	{
		content.load(tex[1], path, texname[1], texinfo);
		textures.insert(tex[1]);
	}
	else
	{
		tex[1] = content.getFactory<Texture>().getZero();
	}
	if (texname.size() > 2)
	{
		// don't compress normal map
		texinfo.compress = false;
		content.load(tex[2], path, texname[2], texinfo);
		textures.insert(tex[2]);
	}
	else
	{
		tex[2] = content.getFactory<Texture>().getZero();
	}
	drawable.SetTextures(tex[0]->GetId(), tex[1]->GetId(), tex[2]->GetId());

	// set mesh
	std::tr1::shared_ptr<Model> mesh;
	content.load(mesh, path, meshname);

	std::string scalestr;
	if (cfg.get("scale", scalestr) &&
		!content.get(mesh, path, meshname + scalestr))
	{
		Vec3 scale;
		std::stringstream s(scalestr);
		s >> scale;

		VertexArray meshva = mesh->GetVertexArray();
		meshva.Scale(scale[0], scale[1], scale[2]);
		content.load(mesh, path, meshname + scalestr, meshva);
	}
	drawable.SetModel(*mesh);
	models.insert(mesh);

	// set color
	Vec4 col(1);
	if (cfg.get("color", col))
	{
		drawable.SetColor(col[0], col[1], col[2], col[3]);
	}

	// set node
	SceneNode * node = &topnode;
	if (nodeptr != 0)
	{
		if (!nodeptr->valid())
		{
			*nodeptr = topnode.AddNode();
			assert(nodeptr->valid());
		}
		node = &topnode.GetNode(*nodeptr);
	}

	Vec3 pos, rot;
	if (cfg.get("position", pos) | cfg.get("rotation", rot))
	{
		if (node == &topnode)
		{
			// position relative to parent, create child node
			keyed_container <SceneNode>::handle nodehandle = topnode.AddNode();
			node = &topnode.GetNode(nodehandle);
		}
		node->GetTransform().SetTranslation(pos);
		node->GetTransform().SetRotation(Quat(rot[0]/180*M_PI, rot[1]/180*M_PI, rot[2]/180*M_PI));
	}

	// set drawable
	keyed_container<Drawable>::handle drawtemp;
	keyed_container<Drawable>::handle * draw = &drawtemp;
	if (drawptr != 0) draw = drawptr;

	std::string drawtype;
	if (cfg.get("draw", drawtype))
	{
		if (drawtype == "emissive")
		{
			drawable.SetDecal(true);
			*draw = node->GetDrawlist().lights_emissive.insert(drawable);
		}
		else if (drawtype == "transparent")
		{
			*draw = node->GetDrawlist().normal_blend.insert(drawable);
		}
	}
	else
	{
		*draw = node->GetDrawlist().car_noblend.insert(drawable);
	}

	return true;
}

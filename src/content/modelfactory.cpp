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

#include "modelfactory.h"
#include "graphics/model_joe03.h"
#include <fstream>

Factory<Model>::Factory() :
	m_default(new Model()),
	m_vbo(false)
{
	// ctor
}

void Factory<Model>::init(bool use_vbo)
{
	m_vbo = use_vbo;

	// init default model
	std::stringstream error;
	VertexArray va;
	va.SetToUnitCube();
	va.Scale(0.5, 0.5, 0.5);
	m_default->Load(va, error, !m_vbo);
}

template <>
bool Factory<Model>::create(
	std::tr1::shared_ptr<Model>& sptr,
	std::ostream& error,
	const std::string& basepath,
	const std::string& path,
	const std::string& name,
	const empty&)
{
	const std::string abspath = basepath + "/" + path + "/" + name;
	if (std::ifstream(abspath.c_str()))
	{
		std::tr1::shared_ptr<ModelJoe03> temp(new ModelJoe03());
		if (temp->Load(abspath, error, !m_vbo))
		{
			sptr = temp;
			return true;
		}
	}
	return false;
}

template <>
bool Factory<Model>::create(
	std::tr1::shared_ptr<Model>& sptr,
	std::ostream& error,
	const std::string& basepath,
	const std::string& path,
	const std::string& name,
	const JoePack& pack)
{
	std::tr1::shared_ptr<ModelJoe03> temp(new ModelJoe03());
	if (temp->Load(name, error, !m_vbo, &pack))
	{
		sptr = temp;
		return true;
	}
	return false;
}

template <>
bool Factory<Model>::create(
	std::tr1::shared_ptr<Model>& sptr,
	std::ostream& error,
	const std::string& basepath,
	const std::string& path,
	const std::string& name,
	const VertexArray& varray)
{
	std::tr1::shared_ptr<Model> temp(new Model());
	if (temp->Load(varray, error, !m_vbo))
	{
		sptr = temp;
		return true;
	}
	return false;
}

const std::tr1::shared_ptr<Model> & Factory<Model>::getDefault() const
{
	return m_default;
}

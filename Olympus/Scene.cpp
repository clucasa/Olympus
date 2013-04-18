#include "Scene.h"

Scene::Scene(vector<Renderable*>* renderables, ID3D11Device *dev, ID3D11DeviceContext *devcon, Apex* apex)
{
	std::ifstream fin("Scene/scene.txt");
    if(!fin)
    {
        MessageBox(0, "Scene/scene.txt not found.", 0, 0);
        return;
    }

	vector<string> filenames;
	std::vector<string> elems;
	string lines, olines;
	string item;
	int numFiles = 0;
	while(getline(fin, lines))
	{
		if(!lines.length()) continue; //skip empty
		if (lines[0] == '#') //skip comments
		{
			continue;
		}
		else if (lines[0] == 'o') 
		{

			string* sline;
			sline = &lines;
			stringstream ss(*sline);
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}
			filenames.push_back(elems[1]);
			numFiles++;
			elems.clear();
		}
	}
	fin.close();

    string model;
	vector<string> textures;
	vector<string> normals;
	vector<ObjectInfo> objInfos;

    for(int i = 0; i < numFiles; i++)
    {
		std::ifstream oin(filenames[i]);
		if(!oin)
		{
			MessageBox(0, "object.txt not found.", 0, 0);
			return;
		}

		while(getline(oin, olines))
		{
			if(!olines.length()) continue; //skip empty
			if (olines[0] == '#') //skip comments
			{
				continue;
			}
			else if (olines[0] == 'm') 
			{
				stringstream ss(olines);
				while(std::getline(ss, item, ' ')) {
					elems.push_back(item);
				}
				model = elems[1];
				elems.clear();
			}
			else if (olines[0] == 't') 
			{
				string* sline;
				sline = &olines;
				stringstream ss(*sline);
				while(std::getline(ss, item, ' ')) {
					elems.push_back(item);
				}
				textures.push_back(elems[1]);
				elems.clear();
			}
			else if (olines[0] == 'n') 
			{
				string* sline;
				sline = &olines;
				stringstream ss(*sline);
				while(std::getline(ss, item, ' ')) {
					elems.push_back(item);
				}
				normals.push_back(elems[1]);
				elems.clear();
			}
			else if (olines[0] == 'i') 
			{
				string* sline;
				sline = &olines;
				stringstream ss(*sline);
				while(std::getline(ss, item, ' ')) {
					elems.push_back(item);
				}
				ObjectInfo object;
				object.x = ::atof(elems[1].c_str());
				object.y = ::atof(elems[2].c_str());
				object.z = ::atof(elems[3].c_str());
				object.sx = ::atof(elems[4].c_str());
				object.sy = ::atof(elems[5].c_str());
				object.sz = ::atof(elems[6].c_str());
				object.rx = ::atof(elems[7].c_str());
				object.ry = ::atof(elems[8].c_str());
				object.rz = ::atof(elems[9].c_str());

				objInfos.push_back(object);
				elems.clear();
			}
		}
		fin.close();

		vector<LPCSTR> texts;
		vector<LPCSTR> norms;
		for(int k = 0; k < textures.size(); k++)
		{
			texts.push_back(textures[k].c_str());
		}
		for(int k = 0; k < normals.size(); k++)
		{
			norms.push_back(normals[k].c_str());
		}
		Object* object = new Object();
		
		char *modelname = new char[model.length() + 1];
		strcpy(modelname, model.c_str());

		object->objLoad(modelname, &texts, &norms, dev, devcon, apex );
		
		for(int k = 0; k < objInfos.size(); k++)
		{
			object->AddInstance(objInfos[k]);
		}
		renderables->push_back(object);

        textures.clear();
        normals.clear();
        objInfos.clear();
        model = "";
    }
}

Scene::~Scene()
{

}
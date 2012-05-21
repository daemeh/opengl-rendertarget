#pragma once

// standard c++ headers
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class RenderTarget
{
public:
	RenderTarget(int width, int height);
	~RenderTarget();

	void Clear();
	void BeginAdd(bool useDepth=true);
	void AddComponent (const string& name, GLint internalFormat, GLenum sourceFormat, GLenum type, GLfloat textureAddress, GLint textureFilter, GLenum attachment);
	void EndAdd();

	void Set();

	GLuint GetTexture(const string& component) const;
	GLenum GetAttachment(const string& component) const;
	GLuint GetAttachmentIndex(const string& component) const;

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

	static void Resolve();

private:
	struct AttachmentData
	{
		AttachmentData()
		{
			m_TextureID = 0;
			m_Buffer = GL_INVALID_ENUM;
			m_Index = -1;
		}
		AttachmentData(GLuint texture, GLenum buffer, GLuint index)
		{
			m_TextureID = texture;
			m_Index = index;
			m_Buffer = buffer;
		}

		GLuint m_TextureID;
		GLenum m_Buffer;
		GLuint m_Index;
	};

	bool m_Adding;
	bool m_UseDepth;

	GLuint m_FBO;
	GLuint m_RBO;
		
	vector<GLenum> m_Buffers;
	vector<GLuint> m_TexIDs;

	typedef unordered_map<string, AttachmentData> DataMap;
	DataMap m_Data;
		
	int m_Width;
	int m_Height;

	int m_CurrentIndex;
};
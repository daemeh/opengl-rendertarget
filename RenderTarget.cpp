#include "RenderTarget.h"

string ToLower(const string& str)
{
	string lowered = str;
	transform(lowered.begin(), lowered.end(),lowered.begin(), tolower );
	return lowered;
}

// helper method to check FBO for errors
void checkFramebufferStatus()
{
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
    case GL_FRAMEBUFFER_COMPLETE:
        break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        throw exception("FBO Building failed! Unsupported framebuffer format.");

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        throw exception("FBO Building failed! Missing attachment.");

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        throw exception("FBO Building failed! Attachment type error.");

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        throw exception("FBO Building failed! Missing draw buffer.");

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        throw exception("FBO Building failed! Missing read buffer.");

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        throw exception("FBO Building failed! Attached images must have the same number of samples.");

    default:
        throw exception("FBO Building failed! Fatal error.");
    }
}
//

RenderTarget::RenderTarget(int width, int height) : m_FBO(0), m_Adding(false), m_CurrentIndex(0), m_RBO(0)
{
	m_Width = width;
	m_Height = height;
}

RenderTarget::~RenderTarget()
{
	Clear();
}

void RenderTarget::Clear()
{
	RenderTarget::Resolve();

	glBindTexture(GL_TEXTURE_2D, 0);

	m_CurrentIndex = 0;

	vector<GLuint>::iterator itTextures;
	for (itTextures=m_TexIDs.begin(); itTextures != m_TexIDs.end(); itTextures++)
	{
		if (*itTextures > 0)
			glDeleteTextures(1, &(*itTextures));
	}

	if (m_FBO > 0)
		glDeleteFramebuffers (1, &m_FBO);
	m_FBO = 0;

	if (m_RBO > 0)
		glDeleteRenderbuffers (1, &m_RBO);
	m_RBO = 0;

	m_Buffers.clear();
	m_TexIDs.clear();
	m_Data.clear();

	m_Adding = false;
}

void RenderTarget::BeginAdd(bool useDepth)
{
	Clear();

	m_UseDepth = useDepth;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	if (m_UseDepth)
	{
		glGenRenderbuffers(1, &m_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	glEnable(GL_TEXTURE_2D);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	m_Adding = true;
}

void RenderTarget::EndAdd()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	// check framebuffer for errors
	checkFramebufferStatus();

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_Adding = false;
}

void RenderTarget::AddComponent(const string& name, GLint internalFormat, GLenum sourceFormat, GLenum type, GLfloat textureAddress, GLint textureFilter, GLenum attachment)
{
	if (!m_Adding)
		throw exception("Cannot add a render-target component outside a BeginAdd/EndAdd block!");

	string loweredName = ToLower(name);

	// check if this name already exists
	DataMap::const_iterator it = m_Data.find(loweredName);
	if (it != m_Data.end())
	{
		throw exception ("Component " + loweredName + " for this FBO already exists!");
	}

	GLuint texid = 0;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	// texture setup
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureAddress);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureAddress);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, sourceFormat, type, NULL);
	
	// binding attachment to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texid, 0);

	// keep track of our attachments and textures
	m_Buffers.push_back(attachment);

	m_TexIDs.push_back(texid);

	m_Data[loweredName] = AttachmentData(texid, attachment, m_CurrentIndex++);
}

void RenderTarget::Set()
{
	if (m_FBO > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glDrawBuffers(m_Buffers.size(), &m_Buffers[0]);
	}
}

void RenderTarget::Resolve()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint RenderTarget::GetAttachmentIndex(const string& component) const
{
	string loweredName = ToLower(component);
	DataMap::const_iterator it = m_Data.find(loweredName);
	if (it == m_Data.end())
		return -1;
	return (*it).second.m_Index;
}

GLuint RenderTarget::GetTexture(const string& component) const
{
	string loweredName = ToLower(component);
	DataMap::const_iterator it = m_Data.find(loweredName);
	if (it == m_Data.end())
		return 0;
	return (*it).second.m_TextureID;
}

GLenum RenderTarget::GetAttachment(const string& component) const
{
	string loweredName = ToLower(component);
	DataMap::const_iterator it = m_Data.find(loweredName);
	if (it == m_Data.end())
		return GL_INVALID_ENUM;
	return (*it).second.m_Buffer;
}
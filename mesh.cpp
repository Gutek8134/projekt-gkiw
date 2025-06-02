#include "mesh.h"
#include <iostream>
// liblodepng-dev
#include <lodepng.h>
#include <glm/gtc/type_ptr.hpp>

#define small_texture 0

Mesh::Mesh(aiMesh *mesh, const aiScene *scene)
{
    name = mesh->mName.C_Str();
    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
        aiVector3D vertex = mesh->mVertices[i];
        vertex_positons.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));
    }

    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
        aiVector3D normal = mesh->mNormals[i];
        vertex_normals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));
    }

    for (int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        // Faces are triangulated, which means each has 3 vertices, yay
        faces.push_back(glm::ivec3(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
    }

    has_texture_coordinates = mesh->HasTextureCoords(0);
    if (has_texture_coordinates)
    {
        for (int i = 0; i < mesh->mNumVertices; ++i)
        {
            aiVector3D coords = mesh->mTextureCoords[0][i];
            texture_coordinates.push_back(glm::vec2(coords.x, coords.y));
        }

        aiString path;
        aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0 && mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
        {
            diffuse_texture = readTexture(path.C_Str());
            // std::cout << "Diffuse: " << path.C_Str() << " ID " << diffuse_texture << std::endl;
        }
        if (mat->GetTextureCount(aiTextureType_SHININESS) > 0 && mat->GetTexture(aiTextureType_SHININESS, 0, &path) == AI_SUCCESS)
        {
            // std::cout << "Roughness: " << path.C_Str() << std::endl;
            roughness_texture = readTexture(path.C_Str());
        }
    }

    initialize_draw_vertices();
    initialize_draw_texture_coordinates();
}

void Mesh::draw(ShaderProgram *sp, glm::mat4 P, glm::mat4 V, glm::mat4 M)
{
    // static bool t = false;
    // if (!t)
    // {
    //     for (const auto &v : draw_vertices)
    //     {
    //         std::cout << v.x << " " << v.y << " " << v.z << std::endl;
    //     }
    //     std::cout << vertex_positons.size() << std::endl;
    //     t = true;
    // }

    sp->use();

    glUniformMatrix4fv(sp->getUniformLocation("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->getUniformLocation("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->getUniformLocation("M"), 1, false, glm::value_ptr(M));

    glEnableVertexAttribArray(sp->getAttributeLocation("vertex"));
    // Vector >>> array
    glVertexAttribPointer(sp->getAttributeLocation("vertex"), 4, GL_FLOAT, false, 0, draw_vertices.data());

    glDrawArrays(GL_TRIANGLES, 0, draw_vertices.size());

    glDisableVertexAttribArray(sp->getAttributeLocation("vertex"));
}

void Mesh::drawTextured(ShaderProgram *sp, glm::mat4 P, glm::mat4 V, glm::mat4 M)
{
    sp->use();

    glUniformMatrix4fv(sp->getUniformLocation("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->getUniformLocation("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->getUniformLocation("M"), 1, false, glm::value_ptr(M));

    glEnableVertexAttribArray(sp->getAttributeLocation("vertex"));
    // Vector >>> array
    glVertexAttribPointer(sp->getAttributeLocation("vertex"), 4, GL_FLOAT, false, 0, draw_vertices.data());

    glEnableVertexAttribArray(sp->getAttributeLocation("texCoord"));
    glVertexAttribPointer(sp->getAttributeLocation("texCoord"), 2, GL_FLOAT, false, 0, draw_texture_coordinates.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture);
    glUniform1i(sp->getUniformLocation("tex"), 0);

    glDrawArrays(GL_TRIANGLES, 0, draw_vertices.size());

    glDisableVertexAttribArray(sp->getAttributeLocation("vertex"));
    glDisableVertexAttribArray(sp->getAttributeLocation("texCoord"));
}

void Mesh::drawTexturedShaded(ShaderProgram *sp, glm::mat4 P, glm::mat4 V, glm::mat4 M, glm::vec4 light_position)
{
    sp->use();

    glUniform4f(sp->getUniformLocation("lightPosition"), light_position.x, light_position.y, light_position.z, light_position.w);
    glUniformMatrix4fv(sp->getUniformLocation("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->getUniformLocation("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->getUniformLocation("M"), 1, false, glm::value_ptr(M));

    glEnableVertexAttribArray(sp->getAttributeLocation("vertex"));
    // Vector >>> array
    glVertexAttribPointer(sp->getAttributeLocation("vertex"), 4, GL_FLOAT, false, 0, draw_vertices.data());

    glEnableVertexAttribArray(sp->getAttributeLocation("texCoord"));
    glVertexAttribPointer(sp->getAttributeLocation("texCoord"), 2, GL_FLOAT, false, 0, draw_texture_coordinates.data());

    glEnableVertexAttribArray(sp->getAttributeLocation("normal"));
    glVertexAttribPointer(sp->getAttributeLocation("normal"), 4, GL_FLOAT, false, 0, draw_normals.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture);
    glUniform1i(sp->getUniformLocation("tex"), 0);

#if small_texture == 0
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, roughness_texture);
    glUniform1i(sp->getUniformLocation("rough"), 1);
#endif

    glDrawArrays(GL_TRIANGLES, 0, draw_vertices.size());

    glDisableVertexAttribArray(sp->getAttributeLocation("vertex"));
    glDisableVertexAttribArray(sp->getAttributeLocation("texCoord"));
    glDisableVertexAttribArray(sp->getAttributeLocation("normal"));
}

GLuint readTexture(const char *filename)
{
    GLuint tex;
    glActiveTexture(GL_TEXTURE0);

    // Read into computers memory
    std::vector<unsigned char> image; // Allocate memory
    unsigned width, height;           // Variables for image size
    // Read the image
    unsigned error = lodepng::decode(image, width, height,
#if small_texture == 1
                                     "bricks.png"
#else
                                     filename
#endif
    );
    if (error)
        std::cout << "LODEPNG ERROR " << error << std::endl;

    // Import to graphics card memory
    glGenTextures(1, &tex);            // Initialize one handle
    glBindTexture(GL_TEXTURE_2D, tex); // Activate handle
    // Copy image to graphics cards memory reprezented by the active handle
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)image.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return tex;
}

void Mesh::initialize_draw_vertices()
{
    draw_vertices = {};
    draw_normals = {};
    for (auto const &face : faces)
    {
        draw_vertices.push_back(vertex_positons[face[0]]);
        draw_vertices.push_back(vertex_positons[face[1]]);
        draw_vertices.push_back(vertex_positons[face[2]]);
        draw_normals.push_back(vertex_normals[face[0]]);
        draw_normals.push_back(vertex_normals[face[1]]);
        draw_normals.push_back(vertex_normals[face[2]]);
    }
}
void Mesh::initialize_draw_texture_coordinates()
{
    if (!has_texture_coordinates)
        return;

    draw_texture_coordinates = {};
    for (auto const &face : faces)
    {
        draw_texture_coordinates.push_back(texture_coordinates[face[0]]);
        draw_texture_coordinates.push_back(texture_coordinates[face[1]]);
        draw_texture_coordinates.push_back(texture_coordinates[face[2]]);
    }
}

Mesh::~Mesh()
{
    if (has_texture_coordinates)
        glDeleteTextures(1, &diffuse_texture);
}
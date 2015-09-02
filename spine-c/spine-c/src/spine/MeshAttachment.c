/******************************************************************************
 * Spine Runtimes Software License v2.5
 *
 * Copyright (c) 2013-2016, Esoteric Software
 * All rights reserved.
 *
 * You are granted a perpetual, non-exclusive, non-sublicensable, and
 * non-transferable license to use, install, execute, and perform the Spine
 * Runtimes software and derivative works solely for personal or internal
 * use. Without the written permission of Esoteric Software (see Section 2 of
 * the Spine Software License Agreement), you may not (a) modify, translate,
 * adapt, or develop new applications using the Spine Runtimes or otherwise
 * create derivative works or improvements of the Spine Runtimes or (b) remove,
 * delete, alter, or obscure any trademarks or any copyright, trademark, patent,
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 *
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
 * USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <spine/MeshAttachment.h>
#include <spine/extension.h>

void _spMeshAttachment_dispose (spAttachment* attachment) {
	spMeshAttachment* self = SUB_CAST(spMeshAttachment, attachment);
	FREE(self->path);
	FREE(self->uvs);
	if (!self->parentMesh) {
		_spVertexAttachment_deinit(SUPER(self));
		FREE(self->regionUVs);
		FREE(self->triangles);
		FREE(self->edges);
	} else
		_spAttachment_deinit(attachment);
	FREE(self);
}

spAttachment* _spMeshAttachment_clone (const spAttachment* attachment) {
	const spMeshAttachment* const self = SUB_CAST(spMeshAttachment, attachment);
	spMeshAttachment* const result = NEW(spMeshAttachment);
	_spAttachment_init(SUPER(SUPER(result)), SUPER(SUPER(self))->name, SP_ATTACHMENT_MESH, _spMeshAttachment_dispose, _spMeshAttachment_clone);

        result->rendererObject = self->rendererObject;
        result->regionOffsetX = self->regionOffsetX;
        result->regionOffsetY = self->regionOffsetY;
        result->regionWidth = self->regionWidth;
        result->regionHeight = self->regionHeight;
        result->regionOriginalWidth = self->regionOriginalWidth;
        result->regionOriginalHeight = self->regionOriginalHeight;
        result->regionU = self->regionU;
        result->regionV = self->regionV;
        result->regionU2 = self->regionU2;
        result->regionV2 = self->regionV2;
        result->regionRotate = self->regionRotate;

        MALLOC_STR(result->path, self->path);

        const int vertexCount = SUPER(self)->verticesCount;
        MALLOC_COPY(result->uvs, self->uvs, float, vertexCount);

        result->r = self->r;
        result->g = self->g;
        result->b = self->b;
        result->a = self->a;

        result->inheritDeform = self->inheritDeform;
        result->width = self->width;
        result->height = self->height;

        if (self->parentMesh) {
            spMeshAttachment_setParentMesh(result, self->parentMesh);
        } else {
            _spVertexAttachment_cloneFields(SUPER(self), SUPER(result));

            MALLOC_COPY(result->regionUVs, self->uvs, float, vertexCount);

            const int triangleCount = self->trianglesCount;
            result->trianglesCount = triangleCount;
            MALLOC_COPY(result->triangles, self->triangles, unsigned short, triangleCount);

            result->hullLength = self->hullLength;
        
            const int edgeCount = self->edgesCount;
            result->edgesCount = edgeCount;
            MALLOC_COPY(result->edges, self->edges, int, edgeCount);
        }        

        if (attachment->attachmentLoader) {
            spAttachmentLoader_configureClonedAttachment (attachment->attachmentLoader, SUPER_CAST(spAttachment, result));
        }
        
        return SUPER_CAST(spAttachment, result);
}

spMeshAttachment* spMeshAttachment_create (const char* name) {
	spMeshAttachment* self = NEW(spMeshAttachment);
	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_MESH, _spMeshAttachment_dispose, _spMeshAttachment_clone);
	return self;
}

void spMeshAttachment_updateUVs (spMeshAttachment* self) {
	int i;
	float width = self->regionU2 - self->regionU, height = self->regionV2 - self->regionV;
	int verticesLength = SUPER(self)->worldVerticesLength;
	FREE(self->uvs);
	self->uvs = MALLOC(float, verticesLength);
	if (self->regionRotate) {
		for (i = 0; i < verticesLength; i += 2) {
			self->uvs[i] = self->regionU + self->regionUVs[i + 1] * width;
			self->uvs[i + 1] = self->regionV + height - self->regionUVs[i] * height;
		}
	} else {
		for (i = 0; i < verticesLength; i += 2) {
			self->uvs[i] = self->regionU + self->regionUVs[i] * width;
			self->uvs[i + 1] = self->regionV + self->regionUVs[i + 1] * height;
		}
	}
}

void spMeshAttachment_computeWorldVertices (spMeshAttachment* self, spSlot* slot, float* worldVertices) {
	spVertexAttachment_computeWorldVertices(SUPER(self), slot, worldVertices);
}

void spMeshAttachment_setParentMesh (spMeshAttachment* self, spMeshAttachment* parentMesh) {
	CONST_CAST(spMeshAttachment*, self->parentMesh) = parentMesh;
	if (parentMesh) {

		self->super.bones = parentMesh->super.bones;
		self->super.bonesCount = parentMesh->super.bonesCount;

		self->super.vertices = parentMesh->super.vertices;
		self->super.verticesCount = parentMesh->super.verticesCount;

		self->regionUVs = parentMesh->regionUVs;

		self->triangles = parentMesh->triangles;
		self->trianglesCount = parentMesh->trianglesCount;

		self->hullLength = parentMesh->hullLength;
		
		self->super.worldVerticesLength = parentMesh->super.worldVerticesLength;

		self->edges = parentMesh->edges;
		self->edgesCount = parentMesh->edgesCount;

		self->width = parentMesh->width;
		self->height = parentMesh->height;
	}
}

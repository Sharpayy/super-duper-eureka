#include "Bezier.h"

BezierRenderer::BezierRenderer(Program program, uint32_t cnt, float quality_ = 2.0f)
{
	quality = quality_;
	maxCount = cnt;
	count = 0;
	bezierProgram = program;
	color = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	bezierVertex = VertexBuffer();
	bezierData = Buffer<GL_ARRAY_BUFFER>(cnt * sizeof(BezierCurveParameters), NULL, GL_DYNAMIC_DRAW);
	bezierVertex.bind();
	bezierData.bind();
	bezierVertex.addAttrib(GL_FLOAT, 0, 2, 2 * 4, 0);

	bezierVertex.enableAttrib(0);

	bezierProgram.use();

	glPatchParameteri(GL_PATCH_VERTICES, BEZIER_PATCH_AMOUNT);
	ulQuality = glGetUniformLocation(bezierProgram.id, "BezierQuality");
	ulColor = glGetUniformLocation(bezierProgram.id, "BezierColor");

	glUniform1f(ulQuality, quality);
	glUniform4fv(ulColor, 1, (float*)&color.x);
}

void BezierRenderer::Render(uint32_t amount = 0)
{
	uint32_t renderAmount;
	if (amount == 0)
		renderAmount = count;
	else
		renderAmount = amount;

	bezierProgram.use();
	bezierVertex.bind();

	glDrawArrays(GL_PATCHES, 0, renderAmount * BEZIER_PATCH_AMOUNT);
}

void BezierRenderer::UpdateData(BezierCurveParameters* data, uint32_t amount, uint32_t amountOffset)
{
	if (amount + amountOffset > maxCount)
		return;

	count = amountOffset + amount;
	bezierData.bind();
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(BezierCurveParameters) * amountOffset, sizeof(BezierCurveParameters) * amount, data);

}

void BezierRenderer::SetQuality(float quality_)
{
	bezierProgram.use();
	glUniform1f(ulQuality, quality);
}

void BezierRenderer::SetColor(vec4 color_)
{
	bezierProgram.use();
	glUniform4fv(ulColor, 1, (float*)&color.x);
}

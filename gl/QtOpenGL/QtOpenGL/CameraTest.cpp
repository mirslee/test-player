
#include "CameraTest.h"


CameraTest::CameraTest()
{
	////�����λ��
	//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	////���������
	//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	//glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	////����  ������
	//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	////����
	//glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

	////�۲����
	//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), /*�����λ��*/
	//	glm::vec3(0.0f, 0.0f, 0.0f), /*Ŀ��λ��*/
	//	glm::vec3(0.0f, 1.0f, 0.0f)); /*������*/

	////�������ת
	//float radius = 10.0f;
	//float camX = sin(glfwGetTime()) * radius;
	//float camZ = cos(glfwGetTime()) * radius;
	//glm::mat4 view2 = glm::lookAt(glm::vec3(camX,0.0,camZ),
	//	glm::vec3(0.0f, 0.0f, 0.0f), 
	//	glm::vec3(0.0f, 1.0f, 0.0f));

	////�����ƶ�
	//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	//glm::mat4 view3 = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

}


CameraTest::~CameraTest()
{
}

void CameraTest::prepare()
{
}
void CameraTest::flush()
{

}
void CameraTest::release()
{

}
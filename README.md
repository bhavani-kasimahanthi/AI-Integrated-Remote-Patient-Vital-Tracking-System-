INTRODUCTION  
Access to immediate medical care is crucial in critical situations such as battlefields, disaster zones, 
rural areas, and medical emergencies. While hospitals, emergency response teams, and 
telemedicine services exist, they often serve as reactive solutions rather than proactive measures. 
According to global health reports, delays in medical intervention significantly increase health risk 
rates in emergencies. Many remote and high-risk areas lack adequate healthcare facilities, leaving 
patients vulnerable without timely medical attention. The need for an efficient, real-time health 
tracking and assistance system is more urgent than ever.

This project introduces a remote-controlled health monitoring and medicine delivery system 
designed to provide immediate medical support without the physical presence of healthcare 
workers. The system is equipped with sensors to measure vital signs, an AI-based diagnostic model 
for patient assessment, and a mobile chassis for medicine delivery. With real-time video streaming 
and remote navigation capabilities, it bridges the gap between patients and healthcare providers in 
inaccessible locations. While telemedicine has made healthcare more accessible, integrating AI
driven remote monitoring and medicine delivery takes emergency response to the next level. This 
system alerts the doctor when patient condition is critical through telegram app. Effective 
implementation of this systems can save lives and revolutionize healthcare accessibility in critical 
situations. 

1.Vital Collection and AI-Based Analysis 
The system is designed to remotely monitor patient health by collecting vital signs such as heart 
rate, oxygen saturation (SpO₂), and body temperature using the MAX30100 pulse oximeter and 
DS18B20 temperature sensor. These readings are processed using an Edge Impulse Machine 
Learning (ML) model, which classifies the patient’s condition as Good, Bad, or Critical. This 
allows doctors to prioritize critical cases and make quick medical decisions without being 
physically present. 

2.Chassis Movement and Camera Streaming via Server 
The system is built on a mobile chassis powered by an L298N motor driver, allowing remote 
navigation to reach the patient. The ESP32-CAM module provides real-time video streaming, 
enabling doctors to observe the patient’s condition and control the system remotely via a web-based 
interface. The combination of remote-controlled movement, live video feed, and AI-based 
diagnostics makes this system highly effective for telemedicine, battlefield healthcare, and 
emergency response scenarios. 

AIM 
The aim of this project is to develop a remote-controlled health tracking and medicine delivery 
system for critical situations like battlefields, disaster zones, and rural areas. It uses ESP32-CAM 
for live streaming, MAX30100 & DS18B20 sensors for vitals monitoring, and AI-based analysis 
to classify patient conditions as Good, Bad, or Critical. This system alerts the doctor when the 
condition is critical. This enables timely medical intervention and remote medicine delivery, 
enhancing telemedicine and emergency healthcare. 

MOTIVATION 
During disasters, medical assistance is often delayed due to infrastructure damage and limited 
access to healthcare. The Remote Patient Vital Tracking System enables real-time monitoring of 
patient’s vitals, ensuring timely intervention. The ESP32-CAM Car assists in remote assessment 
and rescue operations, improving emergency response and saving lives. 

PROBLEM STATEMENT 
In emergency situations, remote areas, and high-risk environments, patients often face delays in      
receiving medical care due to the lack of immediate access to doctors. This can be critical in 
battlefields, disaster zones, and quarantine centers, where timely diagnosis and treatment are 
essential. There is a need for a system that can remotely collect patient vitals, analyze their 
condition using AI, and alert doctor alongside assisting in delivering essential medicines, ensuring 
that medical professionals can provide quick and effective care, even from a distance. 

OBJECTIVE 
To design and develop a portable IoT-based remote health monitoring system integrated with 
machine learning that enables real-time patient vital tracking, remote navigation via live video 
streaming, and automated diagnosis and alert mechanisms. The system aims to assist doctors in 
assessing patient conditions from a distance using an ESP32-CAM-enabled chassis and medical 
sensors, thereby improving emergency response and healthcare delivery in remote, rural, and high
risk areas. 

APPLICATIONS 
1. Remote Patient Monitoring – Helps doctors assess patients in rural areas, quarantine zones, 
and disaster-hit locations without physical presence.  
2. Emergency & Disaster Response – Provides real-time health assessment for victims in 
earthquakes, floods, and war zones.  
3. Battlefield Healthcare – Assists military medics by remotely monitoring injured soldiers 
and delivering medicines in combat zones.  
4. Telemedicine & AI-Based Diagnosis – Uses machine learning to classify patient conditions, 
enabling faster medical decisions.  
5. Elderly & Home Healthcare – Can be used in nursing homes to monitor vital signs and 
provide timely medicine delivery.  
6. Smart Hospitals & IoT Healthcare – Integrates into modern hospital systems for automated 
health monitoring and patient tracking.  
7. Post-Surgical Monitoring - Allows continuous tracking of patients’ vitals after surgery, 
alerting doctors to complications like infections or abnormal recovery trends.

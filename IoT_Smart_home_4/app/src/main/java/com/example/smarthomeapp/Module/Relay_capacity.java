package com.example.smarthomeapp.Module;

public class Relay_capacity {
    float energyConsumption, power;
    String name;
    public Relay_capacity(float energyConsumption, float power, String name) {
        this.energyConsumption = energyConsumption;
        this.power = power;
        this.name = name;
    }

    public Relay_capacity() {
        this.energyConsumption = 0;
        this.power = 0;
        this.name = "";
    }

    public String getName2() {
        return name;
    }

    public void setName2(String name) {
        this.name = name;
    }

    public float getPower() {
        return power;
    }

    public void setPower(float power) {
        this.power = power;
    }

    public float getEnergyConsumption() {
        return energyConsumption;
    }

    public void setEnergyConsumption(float energyConsumption) {this.energyConsumption = energyConsumption;}

}

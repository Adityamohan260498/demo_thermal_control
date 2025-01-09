#include <rclcpp/rclcpp.hpp>
#include "thermal_control/srv/cooling.hpp"
#include <functional>
#include <memory>

class CoolingSystemNode : public rclcpp::Node
{
public:
    CoolingSystemNode()
        : Node("cooling_system_server"),
          pump1_active_(false),
          T_water_(4.0) // Water temperature remains constant
    {
        // Service Server
        cooling_service_ = this->create_service<thermal_control::srv::Cooling>(
            "activate_cooling",
            std::bind(&CoolingSystemNode::handle_cooling_request, this, std::placeholders::_1, std::placeholders::_2));

        RCLCPP_INFO(this->get_logger(), "Cooling System Node Initialized");
    }

private:
    bool pump1_active_;
    double T_water_;

    rclcpp::Service<thermal_control::srv::Cooling>::SharedPtr cooling_service_;

    void handle_cooling_request(const thermal_control::srv::Cooling::Request::SharedPtr request,
                                 thermal_control::srv::Cooling::Response::SharedPtr response)
    {
        if (pump1_active_)
        {
            response->success = false;
            response->message = "Cooling system is already active!";
            return;
        }

        double T_coldplate = request->temperature; // Get temperature from client
        double target_temperature = 20.0;         // Define the cooling target (e.g., 20°C)

        RCLCPP_INFO(this->get_logger(), "Received cooling request: Current Temp = %.2f°C, Target Temp = %.2f°C",
                    T_coldplate, target_temperature);

        if (T_coldplate <= target_temperature)
        {
            response->success = false;
            response->message = "Cooling not required. Current temperature is already at or below the target.";
            response->reduced_temperature = T_coldplate;
            return;
        }

        pump1_active_ = true;
        // T_coldplate = simulate_cooling(T_coldplate, target_temperature);
        const double cooling_rate = 0.1; // Rate of cooling proportional to temperature difference
        double temp_difference = T_coldplate - T_water_;
        T_coldplate -= cooling_rate * temp_difference;
        response->reduced_temperature = T_coldplate;
        response->success = true;
        response->message = "Cooling completed successfully.";
        
        // RCLCPP_INFO(this->get_logger(), "Cooling in progress: T_coldplate=%.2f°C", T_coldplate);
        // rclcpp::sleep_for(std::chrono::milliseconds(100));
        

        pump1_active_ = false;
    }

//     double simulate_cooling(double T_coldplate, double target_temperature)
//     {
//         const double cooling_rate = 0.1; // Rate of cooling proportional to temperature difference

//         while (T_coldplate > target_temperature)
//         {
//             double temp_difference = T_coldplate - T_water_;
//             T_coldplate -= cooling_rate * temp_difference;

//             RCLCPP_INFO(this->get_logger(), "Cooling in progress: T_coldplate=%.2f°C", T_coldplate);
//             rclcpp::sleep_for(std::chrono::milliseconds(500));
//         }

//         RCLCPP_INFO(this->get_logger(), "Target temperature reached: %.2f°C", T_coldplate);
//         return T_coldplate;
//     }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CoolingSystemNode>());
    rclcpp::shutdown();
    return 0;
}

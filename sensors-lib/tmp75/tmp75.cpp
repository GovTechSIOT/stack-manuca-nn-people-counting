#include "mbed.h"
#include "tmp75.h"

/** Create a TMP75 object using the specified I2C object
 * @param sda 			mbed I2C interface pin
 * @param scl   		mbed I2C interface pin
 * @param alert_pin		mbed Digital Input interface pin
 * @param i2c_frequency	I2C Frequency (in Hz)
 */
Tmp75::Tmp75(PinName sda, PinName scl, PinName alert_pin, int i2c_frequency)	: _alert(alert_pin), 
	tmp_config_t(), active_(0),
	_i2c(sda, scl) {  _i2c.frequency(i2c_frequency);
}

/** Destructor
 */
Tmp75::~Tmp75()	{
}

/** Get Sensor Name (Overrides SensorType virtual func)
 *
 * @return sensor name in string
 */
std::string Tmp75::GetName()
{
	return "tmp75";
}

/** Get Sensor Data (Overrides SensorType virtual func)
 * 
 * @param   data_list   vector of string pairs (data_name, data_value)
 *
 * @return  enum SensorStatus in SensorType base class
 */
int Tmp75::GetData(std::vector<std::pair<std::string, std::string>>& data_list)
{
	if (!active_) return DISCONNECT;

	int ret = ReadTemp();
	if (ret != Tmp75::TMPACK) return DISCONNECT;

	float tmp_temp = GetTempData();
	std::string amb_temp = ConvertDataToString(tmp_temp);
	data_list.push_back(make_pair("ambient_temp", amb_temp));

	int alert = ReadAlert();
	if (alert == TMPALERT)
	{
		data_list.push_back(make_pair("ambient_temp_alert", ""));
	}
	return DATA_OK;
}

/** Enables Sensor (Overrides SensorType virtual func)
 *
 */
void Tmp75::Enable()
{
	/* Startup configuration */
	tmp_config_t.pol = 1;
	tmp_config_t.tmode = 0;
	tmp_config_t.fqueue = 2;    // 4 faults
	tmp_config_t.cvrate = 3;    // 4.5 Hz
	float interrupt_high_thres = 45.00;
	float interrupt_low_thres = 40.00;
	SetTempHigh(interrupt_high_thres);
	SetTempLow(interrupt_low_thres);

	StartMeasurement();
	active_ = 1;
}

/** Disables Sensor (Overrides SensorType virtual func)
 *
 */
void Tmp75::Disable()
{
	StopMeasurement();
	active_ = 0;
}

/** Resets Sensor (Overrides SensorType virtual func)
 *
 */
void Tmp75::Reset()
{
	HardReset();
	if (active_) Enable();
	else Disable();
}

/** Configure Sensor temperature thresholds
 * 	@param 	t_low	temperature lower threshold (default: 40C)
 * 	@param 	t_high	temperature higher threshold (default: 50C)
 * 
 * 	@return	enum TmpStatus
 */
int Tmp75::Configure(float t_low, float t_high)
{
	int ret;
	ret = SetTempLow(t_low);
	if (ret != TMPACK) return TMPCONFIGFAIL;

	ret = SetTempHigh(t_high);
	if (ret != TMPACK) return TMPCONFIGFAIL;

	ret = ReadTempLow();
	if (ret != TMPACK) return TMPCONFIGFAIL;

	ret = ReadTempHigh();
	if (ret != TMPACK) return TMPCONFIGFAIL;

	if (temp_low_ != t_low) return TMPCONFIGFAIL;
	if (temp_high_ != t_high) return TMPCONFIGFAIL;
	return TMPOK;
}

/** Start One Measurement
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::StartOneMeasurement()
{
	tmp_config_t.oneshot = 1;
	tmp_config_t.shutdown = 1;
	int ret = Tmp75::SetConfigReg();
	if (ret) 
	{
		ret = Tmp75::ReadTemp();
	}
	if (ret) return TMPACK;
	else return TMPNOACK;
}
	
/** Start Continuous Measurement
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::StartMeasurement()
{
	tmp_config_t.oneshot = 0;
	tmp_config_t.shutdown = 0;
	
	int ret = Tmp75::SetConfigReg();
	if (ret) return TMPACK;
	else return TMPNOACK;
}

/** Stop Measurement and go into shutdown mode
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::StopMeasurement()
{
	tmp_config_t.shutdown = 1;
	int ret = Tmp75::SetConfigReg();
	if (ret) return TMPACK;
	else return TMPNOACK;
}

/** Reset Tmp75 internal registers to power-up values
 *
 */
void Tmp75::HardReset()
{
	i2cbuff[0] = 0x06;
	_i2c.write(0x00, i2cbuff, 1, false);

}

/** Set Configuration Settings
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::SetConfigReg()
{
	if (tmp_config_t.cvrate < 0 || tmp_config_t.cvrate > 3)		// if integer is out-of-range (0-3)
	{
		tmp_config_t.cvrate = 0;							// set config integer as 0
	}
	
	if (tmp_config_t.fqueue < 0 || tmp_config_t.fqueue > 3)		// if integer is out-of-range (0-3)
	{
		tmp_config_t.fqueue = 0;							// set config integer as 0
	}

	uint8_t conf_buf = ((tmp_config_t.oneshot<<7) | (tmp_config_t.cvrate<<5) | (tmp_config_t.fqueue<<3) | (tmp_config_t.pol<<2) | (tmp_config_t.tmode<<1) | tmp_config_t.shutdown);

	i2cbuff[0] = TMP75_CMMD_CONFIG_REG;
	i2cbuff[1] = conf_buf;
	i2cbuff[2] = TMP75_CONF_RESERVED;

	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 3, false);
	if(res) return TMPNOACK;

	return TMPACK;
}

/** Configure TMP75 Low Temperature Threshold Register
 * @param val	temperature lower threshold value in float
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::SetTempLow(float val)
{
	if (val > TMP75_UPPER_LIMIT || val < TMP75_LOWER_LIMIT) return TMPCONFIGFAIL;		// do not set register if out of range
	
	uint16_t temp = val / TMP75_RESOLUTION;
	temp <<= 4;		// left shift by 4

	i2cbuff[0] = TMP75_CMMD_TEMP_LOW_REG; 
	i2cbuff[1] = temp >> 8;
	i2cbuff[2] = temp & 255;

	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 3, false);
	if(res) return TMPNOACK;
	temp_low_ = val;
	return TMPACK;
}

/** Configure TMP75 High Temperature Threshold Register
 * @param val	temperature higher threshold value in float
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::SetTempHigh(float val)
{
	if (val > TMP75_UPPER_LIMIT || val < TMP75_LOWER_LIMIT) return TMPCONFIGFAIL;		// do not set register if out of range

	uint16_t temp = val / TMP75_RESOLUTION;
	temp <<= 4;		// left shift by 4

	i2cbuff[0] = TMP75_CMMD_TEMP_HIGH_REG; 
	i2cbuff[1] = temp >> 8;
	i2cbuff[2] = temp & 255;

	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 3, false);
	if(res) return TMPNOACK;
	temp_high_ = val;
	return TMPACK;
}

/** Read TMP75 Configuration Register
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::ReadConfigReg()
{
	i2cbuff[0] = TMP75_CMMD_CONFIG_REG;

	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 1, false);
	if(res) return TMPNOACK;

	res = _i2c.read(TMP75_I2C_ADDR, i2cbuff, 2, false);
	if(res) return TMPNOACK;

	uint8_t config_reg = i2cbuff[0];

	tmp_config_t.oneshot = (config_reg & TMP75_CONF_ONE_SHOT) >> 7;
	tmp_config_t.cvrate = (config_reg & TMP75_CONF_CONV_RATE) >> 5;
	tmp_config_t.fqueue = (config_reg & TMP75_CONF_FAULT_Q) >> 3;
	tmp_config_t.pol = (config_reg & TMP75_CONF_ALRT_POL) >> 2;
	tmp_config_t.tmode = (config_reg & TMP75_CONF_ALRT_MODE) >> 1;
	tmp_config_t.shutdown = (config_reg & TMP75_CONF_SD_MODE);

	return TMPACK;
}

/** Read TMP75 Low Temperature Register
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::ReadTempLow()
{
	i2cbuff[0] = TMP75_CMMD_TEMP_LOW_REG;
	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 1, false);
	if (res) return TMPNOACK;

	res = _i2c.read(TMP75_I2C_ADDR, i2cbuff, 2, false);
	if(res) return TMPNOACK;

	uint16_t stat = (i2cbuff[0] << 8 | i2cbuff[1]);
	temp_low_ = (stat >> 4) * TMP75_RESOLUTION;
	return TMPACK;
}

/** Read TMP75 High Temperature Register
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::ReadTempHigh()
{
	i2cbuff[0] = TMP75_CMMD_TEMP_HIGH_REG;
	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 1, false);
	if (res) return TMPNOACK;

	res = _i2c.read(TMP75_I2C_ADDR, i2cbuff, 2, false);
	if(res) return TMPNOACK;

	uint16_t stat = (i2cbuff[0] << 8 | i2cbuff[1]);
	temp_high_ = (stat >> 4) * TMP75_RESOLUTION;
	return TMPACK;
}

/** Read TMP75 Temperature Measurement Register
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::ReadTemp()
{
	i2cbuff[0] = TMP75_CMMD_READ_TEMP_REG;
	int res = _i2c.write(TMP75_I2C_ADDR, i2cbuff, 1, false);
	if (res) return TMPNOACK;

	res = _i2c.read(TMP75_I2C_ADDR, i2cbuff, 2, false);
	if(res) return TMPNOACK;

	uint16_t stat = (i2cbuff[0] << 8 | i2cbuff[1]);
	temp_data_ = (stat >> 4) * TMP75_RESOLUTION;
	return TMPACK;
}

/** Read TMP75 Alert Digital Input Pin
 *
 * @return enum TmpStatus
 */
uint8_t Tmp75::ReadAlert()
{
	int alert_stat = _alert.read();
	if (alert_stat == tmp_config_t.pol)
	{
		return TMPALERT;
	}
	else
	{
		return TMPOK;
	}
}

/** Returns TMP75 Temperature Measurement Register
 *
 * @return Temperature Data Float
 */
float Tmp75::GetTempData()
{
	return temp_data_;
}

/** Returns TMP75 Temperature Low Register
 *
 * @return Temperature Low Float
 */
float Tmp75::GetTempLow()
{
	return temp_low_;
}

/** Returns TMP75 Temperature High Register
 *
 * @return Temperature High Float
 */
float Tmp75::GetTempHigh()
{
	return temp_high_;
}
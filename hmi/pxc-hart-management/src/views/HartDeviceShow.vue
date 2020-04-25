<template>
  <HartDeviceComponent :device="device"/>
</template>

<script lang="ts">
// @ is an alias to /src
import { Vue, Component, Prop } from 'vue-property-decorator'
import HartDeviceComponent from '@/components/HartDeviceComponent.vue'
import { HartMuxDto, hartServerUrl, updateInterval } from '../types'
import { HartDeviceDto } from '../types/hart-device'
import { HartVarsDto, HartVars } from '../types/hart-vars'

const blankDevice: HartDeviceDto = {
  name: "",
  company: "",
  longTag: "",
  cmdRevLvl: 0,
  manufacturerId: 0,
  privateLabelDistCode: 0,
  profile: 0,
  revision: 0,
  ioCard: 0,
  channel: 0,
  extendedDeviceStatus: []
}

@Component({
  components: {
    HartDeviceComponent
  }
})
export default class HartDeviceShow extends Vue {
  public device: HartDeviceDto = blankDevice
  private polling = 0;

  mounted() {
    this.updateDevice()
    this.polling = setInterval(this.updateDevice, updateInterval)
    
    // pull device from cache if present
    if (localStorage.device) {
        const device = JSON.parse(localStorage.device) as HartDeviceDto
        if (device.ioCard == parseInt(this.$route.params.ioCard, 10)
              && device.channel == parseInt(this.$route.params.channel, 10)) {
            Object.keys(device.vars!).forEach(key => {
                // FIXME
                (device.vars as any)[key]!.lastUpdated = new Date((device.vars as any)[key]!.lastUpdated) // convert the date string back to a Date()
            })
            this.device = device
        }
    }
  }

  beforeDestroy () {
    clearInterval(this.polling)
  }

  public updateDevice() {
    let serialNo = this.$route.params.serialNo
    let ioCard = this.$route.params.ioCard
    let channel = this.$route.params.channel
    let updatedDevice: HartDeviceDto
    this.$http.get(hartServerUrl + '/gw/' + serialNo + '/info').then(res => {
      const data = res.data as HartMuxDto
      data.devices.forEach(device => {
        if (device.ioCard == parseInt(ioCard, 10)
            && device.channel == parseInt(channel, 10)) updatedDevice = device
      })
    }).then(() => {
      if (updatedDevice) {
        this.$http.get(hartServerUrl + '/gw/' + serialNo + '/vars/'
            + updatedDevice.ioCard + '/' + updatedDevice.channel).then(res => {
          const data = res.data as HartVarsDto
          let vars: HartVars = data as HartVars
          Object.keys(vars).forEach(key => {
            // FIXME
            (vars as any)[key]!.lastUpdated = new Date()
          })
          updatedDevice.vars = vars
          this.device = updatedDevice
        })
      }
    })
  }
}
</script>

<style lang="scss" scoped>
</style>
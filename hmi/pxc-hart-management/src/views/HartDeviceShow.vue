<template>
  <div class="container">
    <HartDeviceComponent :device="device"/>
  </div>
</template>

<script lang="ts">
// @ is an alias to /src
import { Vue, Component, Prop } from 'vue-property-decorator'
import HartDeviceComponent from '@/components/HartDeviceComponent.vue'
import { HartDeviceDto, HartMuxDto, hartServerUrl } from '../types'

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
  channel: 0
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
    this.polling = setInterval(this.updateDevice, 2000)
  }

  beforeDestroy () {
      clearInterval(this.polling)
  }

  public updateDevice() {
    this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/info').then((res) => {
        const data = res.data as HartMuxDto
        data.devices.forEach(device => {
            if (device.ioCard == parseInt(this.$route.params.ioCard, 10)
                && device.channel == parseInt(this.$route.params.channel, 10)) this.device = device
        })
    })
  }
}
</script>

<style lang="scss" scoped>
</style>
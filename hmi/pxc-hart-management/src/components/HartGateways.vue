<template>
<table class="table table-striped">
    <router-link :to="'/gateways/' + gw.ip" v-for="gw in gateways" v-bind:key="gw.serialNo">
        <tr>
            {{ gw.ip }}
        </tr>
    </router-link>
</table>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import { HartDeviceDto, hartServerUrl, HartMuxDto, HartGwDto, HartGw } from '../types'

@Component({})
export default class HartGateways extends Vue {
    public gateways: HartGw[] = []
    private polling: number

    mounted () {
        this.updateGateways()
        this.polling = setInterval(this.updateGateways, 2000)
    }

    beforeDestroy () {
        clearInterval(this.polling)
    }

    public updateGateways () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            this.gateways = (res.data as HartGwDto).gateways
            console.log(res.data)
        })
    }
}
</script>

<style lang="scss">
.disabled, .disabled * {
    background-color: #666 !important;
    border-color: #666 !important;
    cursor: default;
}
</style>